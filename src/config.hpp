#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Rule
{
    // Relative to the organized folder, or absolute
    fs::path folder;

    // Lowercase, with the leading dot
    std::vector<std::string> extensions;
};

struct Config
{
    // Checked in order: the first rule with the extension wins
    std::vector<Rule> rules;

    // Folder for files that match no rule; empty = leave them in place
    fs::path other = "Other";

    // Lowercase file name patterns (* and ?) of files that are never moved
    std::vector<std::string> exclude;

    // Problems that don't prevent using the config (e.g. an extension
    // used by two rules)
    std::vector<std::string> warnings;
};

class ConfigError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

// The categories used when there is no config file
Config defaultConfig();

Config loadConfig(const fs::path& file);

// %APPDATA%\file-organizer\config.toml on Windows,
// ~/.config/file-organizer/config.toml elsewhere
fs::path defaultConfigPath();

// Creates a config file containing the default categories
void writeDefaultConfig(const fs::path& file);
