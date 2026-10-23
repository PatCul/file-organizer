#include "organizer.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "journal.hpp"
#include "utils.hpp"

OrganizeSummary organizeDirectory(const fs::path& directory, bool dryRun)
{
    std::map<std::string, std::string> categories = {

        // Documents
        {".txt", "Documents"},
        {".doc", "Documents"},
        {".docx", "Documents"},
        {".pdf", "Documents"},
        {".odt", "Documents"},
        {".rtf", "Documents"},
        {".tex", "Documents"},

        // Spreadsheets
        {".xls", "Spreadsheets"},
        {".xlsx", "Spreadsheets"},
        {".csv", "Spreadsheets"},
        {".ods", "Spreadsheets"},

        // Presentations
        {".ppt", "Presentations"},
        {".pptx", "Presentations"},
        {".odp", "Presentations"},

        // Images
        {".jpg", "Images"},
        {".jpeg", "Images"},
        {".png", "Images"},
        {".gif", "Images"},
        {".bmp", "Images"},
        {".webp", "Images"},
        {".svg", "Images"},
        {".ico", "Images"},
        {".tiff", "Images"},

        // Videos
        {".mp4", "Videos"},
        {".mkv", "Videos"},
        {".avi", "Videos"},
        {".mov", "Videos"},
        {".wmv", "Videos"},
        {".webm", "Videos"},

        // Audio
        {".mp3", "Music"},
        {".wav", "Music"},
        {".flac", "Music"},
        {".aac", "Music"},
        {".ogg", "Music"},
        {".m4a", "Music"},

        // Archives
        {".zip", "Archives"},
        {".rar", "Archives"},
        {".7z", "Archives"},
        {".tar", "Archives"},
        {".gz", "Archives"},
        {".bz2", "Archives"},

        // Programming
        {".cpp", "Code"},
        {".h", "Code"},
        {".hpp", "Code"},
        {".c", "Code"},
        {".cs", "Code"},
        {".java", "Code"},
        {".py", "Code"},
        {".js", "Code"},
        {".ts", "Code"},
        {".php", "Code"},
        {".html", "Code"},
        {".css", "Code"},
        {".sql", "Code"},
        {".json", "Code"},
        {".xml", "Code"},

        // Executables / installers
        {".exe", "Programs"},
        {".msi", "Programs"},

        // Disk images
        {".iso", "Disk Images"},
        {".img", "Disk Images"},

        // Fonts
        {".ttf", "Fonts"},
        {".otf", "Fonts"},
        {".woff", "Fonts"},
        {".woff2", "Fonts"},

        // E-books
        {".epub", "Ebooks"},
        {".mobi", "Ebooks"},

        // Subtitles
        {".srt", "Subtitles"},
        {".ass", "Subtitles"},

        // 3D / CAD
        {".obj", "3D"},
        {".fbx", "3D"},
        {".stl", "3D"},
        {".blend", "3D"},

        // Database
        {".db", "Database"},
        {".sqlite", "Database"},
        {".sqlite3", "Database"},

        // Logs
        {".log", "Logs"},

        // Backups
        {".bak", "Backups"}
    };

    // Files still being downloaded or written by another program:
    // moving them would break the download
    const std::set<std::string> incompleteDownloads = {
        ".crdownload", // Chrome, Edge
        ".part",       // Firefox
        ".download",   // Safari
        ".tmp"
    };

    OrganizeSummary summary;

    // Collect the files first: moving them while iterating the
    // directory would modify it during the iteration
    std::vector<fs::path> files;

    for (const fs::directory_entry& entry : fs::directory_iterator(directory))
    {
        if (!entry.is_regular_file()
            || entry.path().filename() == JOURNAL_FILE_NAME)
        {
            continue;
        }

        if (incompleteDownloads.count(toLower(entry.path().extension().string())))
        {
            summary.skipped++;
            continue;
        }

        files.push_back(entry.path());
    }

    // Opened only when there is something to move, so a run that
    // moves nothing keeps the log of the previous run
    std::optional<JournalWriter> journal;

    if (!dryRun && !files.empty())
    {
        journal.emplace(directory);
    }

    for (const fs::path& file : files)
    {
        // A single failing file (locked, missing permissions...)
        // must not stop the organization of the others
        try
        {
            std::string extension = toLower(file.extension().string());

            auto it = categories.find(extension);

            fs::path categoryFolder;

            if (it == categories.end())
            {
                categoryFolder = directory / "Other";
            }
            else
            {
                categoryFolder = directory / it->second;
            }

            if (!fs::exists(categoryFolder))
            {
                if (!dryRun)
                {
                    fs::create_directory(categoryFolder);
                    journal->recordCreateDirectory(categoryFolder.lexically_relative(directory));
                }
            }

            fs::path newPath = categoryFolder / file.filename();

            newPath = getUniquePath(newPath);

            if (!dryRun)
            {
                fs::rename(file, newPath);
                journal->recordMove(
                    file.lexically_relative(directory),
                    newPath.lexically_relative(directory));
            }
            else
            {
                std::cout << "[DRY RUN] "
                    << file
                    << " -> "
                    << newPath
                    << std::endl;
            }

            summary.moved++;
        }
        catch (const fs::filesystem_error& e)
        {
            summary.failed++;
            summary.errors.push_back(file.filename().u8string() + ": " + e.what());
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
                fs::rename(current, original);
            }
            else
            {
                std::cout << "[DRY RUN] "
                    << entry.to.u8string()
                    << " -> "
                    << entry.from.u8string()
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
            path.stem().string()
            + " ("
            + std::to_string(counter)
            + ")"
            + path.extension().string();

        newPath = path.parent_path() / name;

        counter++;

    } while (fs::exists(newPath));

    return newPath;
}