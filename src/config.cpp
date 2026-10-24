#include "config.hpp"

#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

#include "../third_party/toml.hpp"
#include "utils.hpp"

Config defaultConfig()
{
    Config config;

    config.rules = {
        {"Documents", {".txt", ".doc", ".docx", ".pdf", ".odt", ".rtf", ".tex"}},
        {"Spreadsheets", {".xls", ".xlsx", ".csv", ".ods"}},
        {"Presentations", {".ppt", ".pptx", ".odp"}},
        {"Images", {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", ".svg", ".ico", ".tiff"}},
        {"Videos", {".mp4", ".mkv", ".avi", ".mov", ".wmv", ".webm"}},
        {"Music", {".mp3", ".wav", ".flac", ".aac", ".ogg", ".m4a"}},
        {"Archives", {".zip", ".rar", ".7z", ".tar", ".gz", ".bz2"}},
        {"Code", {".cpp", ".h", ".hpp", ".c", ".cs", ".java", ".py", ".js", ".ts",
                  ".php", ".html", ".css", ".sql", ".json", ".xml"}},
        {"Programs", {".exe", ".msi"}},
        {"Disk Images", {".iso", ".img"}},
        {"Fonts", {".ttf", ".otf", ".woff", ".woff2"}},
        {"Ebooks", {".epub", ".mobi"}},
        {"Subtitles", {".srt", ".ass"}},
        {"3D", {".obj", ".fbx", ".stl", ".blend"}},
        {"Database", {".db", ".sqlite", ".sqlite3"}},
        {"Logs", {".log"}},
        {"Backups", {".bak"}}
    };

    // Hidden Windows system files that must stay in their folder
    config.exclude = {"desktop.ini", "thumbs.db"};

    return config;
}

// Error pointing to the line of the config file that caused it
[[noreturn]] static void fail(const fs::path& file, const toml::node& node, const std::string& message)
{
    throw ConfigError(
        file.u8string() + ":" + std::to_string(node.source().begin.line) + ": " + message);
}

static std::string getString(const fs::path& file, const toml::node& node, const std::string& name)
{
    const toml::value<std::string>* value = node.as_string();

    if (value == nullptr)
    {
        fail(file, node, "'" + name + "' must be a text between quotes");
    }

    return value->get();
}

static const toml::array& getArray(const fs::path& file, const toml::node& node, const char* name)
{
    const toml::array* array = node.as_array();

    if (array == nullptr)
    {
        fail(file, node, "'" + std::string(name) + "' must be a list, e.g. [\"a\", \"b\"]");
    }

    return *array;
}

static fs::path getFolder(const fs::path& file, const toml::node& node, const std::string& name)
{
    std::string text = getString(file, node, name);

    // libstdc++ on Windows reads "\\server\share" as "C:\server\share",
    // so network folders would end up in the wrong place
    if (text.size() >= 2 && (text[0] == '/' || text[0] == '\\') && (text[1] == '/' || text[1] == '\\'))
    {
        fail(file, node, "network folders (\\\\server\\share) are not supported, use a drive letter");
    }

    fs::path folder = fs::u8path(text);

    // On Windows "\Music" (no drive) and "C:Music" (no slash) are
    // neither relative nor absolute: they depend on the current drive
    // or folder, so a typo could create folders in unexpected places
    if ((folder.has_root_name() || folder.has_root_directory()) && !folder.is_absolute())
    {
        fail(file, node, "'" + name + "' must be a relative folder (e.g. \"Images\") "
            "or a complete path (e.g. \"D:/Backup/Images\")");
    }

    fs::path normal = folder.lexically_normal();

    if (normal.empty() || normal == ".")
    {
        fail(file, node, "'" + name + "' can't be the organized folder itself");
    }

    return folder;
}

static std::string getExtension(const fs::path& file, const toml::node& node)
{
    std::string extension = toLower(getString(file, node, "extensions"));

    if (extension.size() < 2 || extension[0] != '.')
    {
        fail(file, node, "the extension '" + extension + "' must start with a dot, e.g. \".pdf\"");
    }

    // fs::path::extension() only returns the last part (".gz" for
    // "file.tar.gz"), so "multi-dot" extensions would never match
    if (extension.find_first_of("./\\*?", 1) != std::string::npos)
    {
        fail(file, node, "the extension '" + extension + "' must be a single extension, e.g. \".gz\"");
    }

    return extension;
}

static Rule getRule(const fs::path& file, const toml::node& node)
{
    const toml::table* table = node.as_table();

    if (table == nullptr)
    {
        fail(file, node, "each rule must be written as a [[rule]] section");
    }

    Rule rule;
    bool hasFolder = false;
    bool hasExtensions = false;

    for (auto&& [key, value] : *table)
    {
        if (key == "folder")
        {
            rule.folder = getFolder(file, value, "folder");
            hasFolder = true;
        }
        else if (key == "extensions")
        {
            for (const toml::node& element : getArray(file, value, "extensions"))
            {
                rule.extensions.push_back(getExtension(file, element));
            }

            hasExtensions = true;
        }
        else
        {
            fail(file, value, "unknown key '" + std::string(key.str())
                + "' in [[rule]] (allowed: folder, extensions)");
        }
    }

    if (!hasFolder)
    {
        fail(file, node, "this [[rule]] has no 'folder'");
    }

    if (!hasExtensions || rule.extensions.empty())
    {
        fail(file, node, "this [[rule]] has no 'extensions'");
    }

    return rule;
}

Config loadConfig(const fs::path& file)
{
    std::ifstream stream(file, std::ios::binary);

    if (!stream)
    {
        throw ConfigError("Cannot open the config file " + file.u8string());
    }

    std::stringstream content;
    content << stream.rdbuf();

    toml::table root;

    try
    {
        root = toml::parse(content.str(), file.u8string());
    }
    catch (const toml::parse_error& e)
    {
        throw ConfigError(
            file.u8string() + ":" + std::to_string(e.source().begin.line) + ": "
            + std::string(e.description()));
    }

    Config config;

    // Without a "rule" section the default categories are used,
    // so a config can change only "other" or "exclude"
    bool hasRules = false;

    // First line where each extension is used, to report duplicates
    std::map<std::string, std::string> extensionOwners;

    for (auto&& [key, value] : root)
    {
        if (key == "other")
        {
            std::string other = getString(file, value, "other");
            config.other = other.empty() ? fs::path() : getFolder(file, value, "other");
        }
        else if (key == "exclude")
        {
            for (const toml::node& element : getArray(file, value, "exclude"))
            {
                std::string pattern = getString(file, element, "exclude");

                if (pattern.empty())
                {
                    fail(file, element, "'exclude' can't contain an empty pattern");
                }

                config.exclude.push_back(toLower(pattern));
            }
        }
        else if (key == "rule")
        {
            if (!value.is_array_of_tables())
            {
                fail(file, value, "rules must be written as [[rule]] sections");
            }

            for (const toml::node& element : *value.as_array())
            {
                Rule rule = getRule(file, element);

                for (const std::string& extension : rule.extensions)
                {
                    auto owner = extensionOwners.find(extension);

                    if (owner != extensionOwners.end())
                    {
                        config.warnings.push_back(
                            file.u8string() + ":" + std::to_string(element.source().begin.line)
                            + ": " + extension + " is already used by the rule for '"
                            + owner->second + "', so this rule will never get it");
                    }
                    else
                    {
                        extensionOwners[extension] = rule.folder.u8string();
                    }
                }

                config.rules.push_back(rule);
            }

            hasRules = true;
        }
        else
        {
            fail(file, value, "unknown key '" + std::string(key.str())
                + "' (allowed: other, exclude, [[rule]])");
        }
    }

    if (!hasRules)
    {
        config.rules = defaultConfig().rules;
    }

    return config;
}

fs::path defaultConfigPath()
{
#ifdef _WIN32
    const wchar_t* appData = _wgetenv(L"APPDATA");

    if (appData != nullptr && *appData != L'\0')
    {
        return fs::path(appData) / "file-organizer" / "config.toml";
    }
#else
    const char* configHome = std::getenv("XDG_CONFIG_HOME");

    if (configHome != nullptr && *configHome != '\0')
    {
        return fs::path(configHome) / "file-organizer" / "config.toml";
    }

    const char* home = std::getenv("HOME");

    if (home != nullptr && *home != '\0')
    {
        return fs::path(home) / ".config" / "file-organizer" / "config.toml";
    }
#endif

    return {};
}

static std::string quoted(const std::string& text)
{
    return "\"" + text + "\"";
}

void writeDefaultConfig(const fs::path& file)
{
    if (fs::exists(file))
    {
        throw ConfigError("The config file " + file.u8string() + " already exists");
    }

    if (file.has_parent_path())
    {
        fs::create_directories(file.parent_path());
    }

    std::ofstream stream(file, std::ios::binary);

    if (!stream)
    {
        throw ConfigError("Cannot create the config file " + file.u8string());
    }

    Config config = defaultConfig();

    stream
        << "# File Organizer configuration\n"
        << "#\n"
        << "# Rules are checked in order: the first rule that contains the\n"
        << "# extension of a file decides where the file goes.\n"
        << "#\n"
        << "# \"folder\" can be relative to the organized folder (e.g. \"Images\")\n"
        << "# or an absolute path (e.g. \"D:/Backup/Images\"). In paths use \"/\",\n"
        << "# \"\\\\\" or single quotes: 'D:\\Backup\\Images'.\n"
        << "#\n"
        << "# Incomplete downloads (.crdownload, .part, .download, .tmp) are\n"
        << "# always skipped.\n"
        << "\n"
        << "# Folder for files that match no rule.\n"
        << "# Empty text (\"\") = leave them where they are.\n"
        << "other = " << quoted(config.other.u8string()) << "\n"
        << "\n"
        << "# Files that are never moved. * = any characters, ? = one character.\n"
        << "# Example: exclude = [\"desktop.ini\", \"*.lnk\", \"keep-*\"]\n"
        << "exclude = [";

    for (size_t i = 0; i < config.exclude.size(); i++)
    {
        stream << (i > 0 ? ", " : "") << quoted(config.exclude[i]);
    }

    stream << "]\n";

    for (const Rule& rule : config.rules)
    {
        stream << "\n[[rule]]\n"
               << "folder = " << quoted(rule.folder.u8string()) << "\n"
               << "extensions = [";

        for (size_t i = 0; i < rule.extensions.size(); i++)
        {
            stream << (i > 0 ? ", " : "") << quoted(rule.extensions[i]);
        }

        stream << "]\n";
    }

    if (!stream)
    {
        throw ConfigError("Cannot write the config file " + file.u8string());
    }
}
