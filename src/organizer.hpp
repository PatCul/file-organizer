#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "config.hpp"

namespace fs = std::filesystem;

struct OrganizeSummary
{
    int moved = 0;
    int skippedDownloads = 0;
    int skippedExcluded = 0;
    int leftInPlace = 0; // No matching rule and no "other" folder
    int failed = 0;
    std::vector<std::string> errors;
};

OrganizeSummary organizeDirectory(const fs::path& directory, const Config& config, bool dryRun);
OrganizeSummary undoLastRun(const fs::path& directory, bool dryRun);
fs::path getUniquePath(const fs::path& path);
