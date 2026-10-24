#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// File, inside the organized folder, that records the operations
// of the last run so they can be undone
inline const std::string JOURNAL_FILE_NAME = ".file-organizer.log";

struct JournalEntry
{
    enum class Type
    {
        Move,
        CreateDirectory
    };

    Type type;

    // Paths are relative to the organized folder, or absolute when
    // outside of it (see pathForJournal)
    fs::path from;
    fs::path to; // Only used by Move
};

class JournalWriter
{
public:
    // Replaces the journal of the previous run
    explicit JournalWriter(const fs::path& directory);

    void recordCreateDirectory(const fs::path& folder);
    void recordMove(const fs::path& from, const fs::path& to);

private:
    void write(const JournalEntry& entry);

    std::ofstream file;
};

// Path relative to the organized folder if it is inside it (so the
// folder can be moved or renamed), absolute otherwise
fs::path pathForJournal(const fs::path& path, const fs::path& directory);

std::vector<JournalEntry> readJournal(const fs::path& directory);
void writeJournal(const fs::path& directory, const std::vector<JournalEntry>& entries);
