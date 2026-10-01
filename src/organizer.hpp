#pragma once

#include <filesystem>

namespace fs = std::filesystem;

void organizeDirectory(const fs::path& directory);
fs::path getUniquePath(const fs::path& path);