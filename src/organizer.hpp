#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct OrganizeSummary
{
    int moved = 0;
    int skipped = 0;
    int failed = 0;
    std::vector<std::string> errors;
};

OrganizeSummary organizeDirectory(const fs::path& directory, bool dryRun);
OrganizeSummary undoLastRun(const fs::path& directory, bool dryRun);
fs::path getUniquePath(const fs::path& path);
