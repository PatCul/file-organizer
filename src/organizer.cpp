#include "organizer.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "journal.hpp"
#include "utils.hpp"

// Files still being downloaded or written by another program:
// moving them would break the download
static const std::set<std::string> INCOMPLETE_DOWNLOADS = {
    ".crdownload", // Chrome, Edge
    ".part",       // Firefox
    ".download",   // Safari
    ".tmp"
};

static bool onSameDrive(const fs::path& first, const fs::path& second)
{
    return toLower(first.root_name().u8string()) == toLower(second.root_name().u8string());
}

// Moves a file, also between different drives (C: -> D:), where
// a simple rename is not possible
static void moveFile(const fs::path& from, const fs::path& to)
{
    std::error_code error;
    fs::rename(from, to, error);

    if (!error)
    {
        return;
    }

    if (error != std::errc::cross_device_link && onSameDrive(from, to))
    {
        throw fs::filesystem_error("cannot move", from, to, error);
    }

    // Copy, then delete the original only if the copy succeeded
    fs::copy_file(from, to);

    std::error_code removeError;
    fs::remove(from, removeError);

    if (removeError)
    {
        // Don't leave the file in two places
        std::error_code ignored;
        fs::remove(to, ignored);

        throw fs::filesystem_error("cannot remove the original after copying it", from, to, removeError);
    }
}

// Creates a folder and its missing parents, recording each of them
// in the journal so the undo can remove them
static void createFolder(const fs::path& folder, const fs::path& directory, JournalWriter& journal)
{
    std::vector<fs::path> missing;

    for (fs::path current = folder; !current.empty() && !fs::exists(current); current = current.parent_path())
    {
        missing.push_back(current);

        if (current == current.parent_path())
        {
            break;
        }
    }

    for (auto it = missing.rbegin(); it != missing.rend(); ++it)
    {
        fs::create_directory(*it);
        journal.recordCreateDirectory(pathForJournal(*it, directory));
    }
}

static std::string displayPath(const fs::path& path, const fs::path& directory)
{
    fs::path shown = pathForJournal(path, directory);
    return shown.make_preferred().u8string();
}

static bool isSameFolder(const fs::path& first, const fs::path& second)
{
    std::error_code error;
    return fs::equivalent(first, second, error);
}

OrganizeSummary organizeDirectory(const fs::path& directory, const Config& config, bool dryRun)
{
    // The first rule that contains an extension wins
    std::map<std::string, fs::path> folderByExtension;

    for (const Rule& rule : config.rules)
    {
        for (const std::string& extension : rule.extensions)
        {
            folderByExtension.insert({extension, rule.folder});
        }
    }

    OrganizeSummary summary;

    struct PlannedMove
    {
        fs::path file;
        fs::path folder;
    };

    // Collect the files first: moving them while iterating the
    // directory would modify it during the iteration
    std::vector<PlannedMove> moves;

    for (const fs::directory_entry& entry : fs::directory_iterator(directory))
    {
        const fs::path& file = entry.path();

        if (!entry.is_regular_file() || file.filename() == JOURNAL_FILE_NAME)
        {
            continue;
        }

        std::string extension = toLower(file.extension().u8string());

        if (INCOMPLETE_DOWNLOADS.count(extension))
        {
            summary.skippedDownloads++;
            continue;
        }

        std::string name = toLower(file.filename().u8string());

        bool excluded = std::any_of(
            config.exclude.begin(),
            config.exclude.end(),
            [&](const std::string& pattern)
            {
                return matchesPattern(name, pattern);
            }
        );

        if (excluded)
        {
            summary.skippedExcluded++;
            continue;
        }

        auto it = folderByExtension.find(extension);
        fs::path folder = it != folderByExtension.end() ? it->second : config.other;

        // An absolute folder ignores the organized directory
        folder = folder.empty() ? fs::path() : directory / folder;

        if (folder.empty() || isSameFolder(folder, directory))
        {
            summary.leftInPlace++;
            continue;
        }

        moves.push_back({file, folder});
    }

    // Opened only when there is something to move, so a run that
    // moves nothing keeps the log of the previous run
    std::optional<JournalWriter> journal;

    if (!dryRun && !moves.empty())
    {
        journal.emplace(directory);
    }

    for (const PlannedMove& move : moves)
    {
        // A single failing file (locked, missing permissions...)
        // must not stop the organization of the others
        try
        {
            if (!dryRun && !fs::exists(move.folder))
            {
                createFolder(move.folder, directory, *journal);
            }

            fs::path newPath = getUniquePath(move.folder / move.file.filename());

            if (!dryRun)
            {
                moveFile(move.file, newPath);
                journal->recordMove(
                    pathForJournal(move.file, directory),
                    pathForJournal(newPath, directory));
            }
            else
            {
                std::cout << "[DRY RUN] "
                    << displayPath(move.file, directory)
                    << " -> "
                    << displayPath(newPath, directory)
                    << std::endl;
            }

            summary.moved++;
        }
        catch (const fs::filesystem_error& e)
        {
            summary.failed++;
            summary.errors.push_back(move.file.filename().u8string() + ": " + e.what());
        }
    }

    return summary;
}

OrganizeSummary undoLastRun(const fs::path& directory, bool dryRun)
{
    std::vector<JournalEntry> entries = readJournal(directory);

    OrganizeSummary summary;

    // Moves that could not be undone stay in the log, so the undo
    // can be retried after fixing the problem
    std::vector<JournalEntry> remaining;

    auto fail = [&](const JournalEntry& entry, const std::string& message)
    {
        summary.failed++;
        summary.errors.push_back(entry.to.u8string() + ": " + message);
        remaining.push_back(entry);
    };

    // Undo in reverse order: files are moved back before their
    // category folders are removed
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
    {
        const JournalEntry& entry = *it;

        if (entry.type == JournalEntry::Type::CreateDirectory)
        {
            // Absolute paths stay absolute when joined
            fs::path folder = directory / entry.from;

            // Folders that still contain files are kept
            if (!dryRun && fs::is_directory(folder) && fs::is_empty(folder))
            {
                std::error_code error;
                fs::remove(folder, error);
            }

            // If some moves failed, the folder may become empty after a
            // retry: keep it in the log so the retry can remove it
            if (fs::is_directory(folder))
            {
                remaining.push_back(entry);
            }

            continue;
        }

        fs::path current = directory / entry.to;
        fs::path original = directory / entry.from;

        try
        {
            if (!fs::exists(current))
            {
                fail(entry, "the file no longer exists");
                continue;
            }

            if (fs::exists(original))
            {
                fail(entry, "a file named " + entry.from.u8string() + " already exists");
                continue;
            }

            if (!dryRun)
            {
                moveFile(current, original);
            }
            else
            {
                std::cout << "[DRY RUN] "
                    << displayPath(current, directory)
                    << " -> "
                    << displayPath(original, directory)
                    << std::endl;
            }

            summary.moved++;
        }
        catch (const fs::filesystem_error& e)
        {
            fail(entry, e.what());
        }
    }

    if (!dryRun)
    {
        if (summary.failed == 0)
        {
            fs::remove(directory / JOURNAL_FILE_NAME);
        }
        else
        {
            std::reverse(remaining.begin(), remaining.end());
            writeJournal(directory, remaining);
        }
    }

    return summary;
}

fs::path getUniquePath(const fs::path& path)
{
    if (!fs::exists(path))
    {
        return path;
    }

    int counter = 1;

    fs::path newPath;

    do
    {
        std::string name =
            path.stem().u8string()
            + " ("
            + std::to_string(counter)
            + ")"
            + path.extension().u8string();

        newPath = path.parent_path() / fs::u8path(name);

        counter++;

    } while (fs::exists(newPath));

    return newPath;
}
