#include "journal.hpp"

#include <sstream>
#include <stdexcept>

// Journal format, one operation per line, fields separated by a tab
// (a character that can't appear in Windows file names):
//
//   MKDIR<TAB>Images
//   MOVE<TAB>photo.jpg<TAB>Images/photo.jpg

static std::string toLine(const JournalEntry& entry)
{
    if (entry.type == JournalEntry::Type::CreateDirectory)
    {
        return "MKDIR\t" + entry.from.generic_u8string();
    }

    return "MOVE\t"
        + entry.from.generic_u8string()
        + "\t"
        + entry.to.generic_u8string();
}

JournalWriter::JournalWriter(const fs::path& directory)
    : file(directory / JOURNAL_FILE_NAME, std::ios::trunc)
{
    if (!file)
    {
        throw std::runtime_error("Cannot create the operations log in " + directory.u8string());
    }
}

void JournalWriter::recordCreateDirectory(const fs::path& folder)
{
    write({JournalEntry::Type::CreateDirectory, folder, {}});
}

void JournalWriter::recordMove(const fs::path& from, const fs::path& to)
{
    write({JournalEntry::Type::Move, from, to});
}

void JournalWriter::write(const JournalEntry& entry)
{
    // Flush every line, so the log is complete even if the program stops
    file << toLine(entry) << std::endl;

    if (!file)
    {
        throw std::runtime_error("Cannot write to the operations log");
    }
}

std::vector<JournalEntry> readJournal(const fs::path& directory)
{
    std::vector<JournalEntry> entries;

    std::ifstream file(directory / JOURNAL_FILE_NAME);

    if (!file)
    {
        return entries;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line))
    {
        lineNumber++;

        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> fields;
        std::stringstream stream(line);
        std::string field;

        while (std::getline(stream, field, '\t'))
        {
            fields.push_back(field);
        }

        if (fields.size() == 2 && fields[0] == "MKDIR")
        {
            entries.push_back({JournalEntry::Type::CreateDirectory, fs::u8path(fields[1]), {}});
        }
        else if (fields.size() == 3 && fields[0] == "MOVE")
        {
            entries.push_back({JournalEntry::Type::Move, fs::u8path(fields[1]), fs::u8path(fields[2])});
        }
        else
        {
            throw std::runtime_error(
                "The operations log is corrupted (line " + std::to_string(lineNumber) + ")");
        }
    }

    return entries;
}

void writeJournal(const fs::path& directory, const std::vector<JournalEntry>& entries)
{
    JournalWriter writer(directory);

    for (const JournalEntry& entry : entries)
    {
        if (entry.type == JournalEntry::Type::CreateDirectory)
        {
            writer.recordCreateDirectory(entry.from);
        }
        else
        {
            writer.recordMove(entry.from, entry.to);
        }
    }
}
