#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include "config.hpp"
#include "journal.hpp"
#include "organizer.hpp"
#include "utils.hpp"

using namespace std;

namespace fs = filesystem;

void printHelp()
{
    cout << "File Organizer" << endl;
    cout << endl;

    cout << "Usage:" << endl;
    cout << "  file-organizer [options] [directory]" << endl;
    cout << endl;

    cout << "Options:" << endl;
    cout << "  --dry-run          Simulate the operation without moving files" << endl;
    cout << "  --undo             Undo the last organization of the directory" << endl;
    cout << "  --config <file>    Use this config file" << endl;
    cout << "  --init-config      Create a config file with the default categories" << endl;
    cout << "  --help             Show this help message" << endl;
    cout << endl;

    fs::path configPath = defaultConfigPath();

    if (!configPath.empty())
    {
        cout << "Default config file:" << endl;
        cout << "  " << configPath.u8string() << endl;
        cout << endl;
    }

    cout << "Examples:" << endl;
    cout << "  file-organizer" << endl;
    cout << "  file-organizer --dry-run" << endl;
    cout << "  file-organizer \"C:\\path\\to\\directory\"" << endl;
    cout << "  file-organizer --dry-run \"C:\\path\\to\\directory\"" << endl;
    cout << "  file-organizer --undo \"C:\\path\\to\\directory\"" << endl;
    cout << "  file-organizer --init-config" << endl;
}

void printSummary(const OrganizeSummary& summary, bool dryRun, bool undo)
{
    cout << endl;
    cout << "Summary" << (dryRun ? " (dry run)" : "") << endl;

    if (undo)
    {
        cout << "  Files " << (dryRun ? "to restore" : "restored") << ": "
             << summary.moved << endl;
    }
    else
    {
        cout << "  Files " << (dryRun ? "to move" : "moved") << ": "
             << summary.moved << endl;

        int skipped = summary.skippedDownloads
            + summary.skippedExcluded
            + summary.leftInPlace;

        cout << "  Skipped: " << skipped << endl;

        if (summary.skippedDownloads > 0)
        {
            cout << "    incomplete downloads: " << summary.skippedDownloads << endl;
        }

        if (summary.skippedExcluded > 0)
        {
            cout << "    excluded: " << summary.skippedExcluded << endl;
        }

        if (summary.leftInPlace > 0)
        {
            cout << "    no matching rule: " << summary.leftInPlace << endl;
        }
    }

    cout << "  Errors: " << summary.failed << endl;

    for (const string& error : summary.errors)
    {
        cerr << "  - " << error << endl;
    }
}

int main(int argc, char* argv[])
{
    Utf8Console console;

    bool dryRun = false;
    bool undo = false;
    bool initConfig = false;
    fs::path configFile;
    fs::path mainFolder;

    vector<string> arguments = getArguments(argc, argv);

    for (size_t i = 0; i < arguments.size(); i++)
    {
        const string& argument = arguments[i];

        if (argument == "--dry-run")
        {
            dryRun = true;
        }
        else if (argument == "--undo")
        {
            undo = true;
        }
        else if (argument == "--config")
        {
            if (i + 1 >= arguments.size())
            {
                cerr << "Error: --config needs the path of the config file." << endl;
                return 1;
            }

            configFile = fs::u8path(arguments[++i]);
        }
        else if (argument == "--init-config")
        {
            initConfig = true;
        }
        else if (argument == "--help")
        {
            printHelp();
            return 0;
        }
        else if (argument.rfind("--", 0) == 0)
        {
            cerr << "Error: Unknown option '" << argument << "'." << endl;
            cerr << "Use 'file-organizer --help' for more information."
                 << endl;

            return 1;
        }
        else if (mainFolder.empty())
        {
            mainFolder = fs::u8path(argument);
        }
        else
        {
            cerr << "Error: Too many arguments." << endl;
            cerr << "Use 'file-organizer --help' for more information."
                 << endl;

            return 1;
        }
    }

    // Without --config the default location is used, if the file exists
    fs::path configPath = configFile.empty() ? defaultConfigPath() : configFile;

    if (initConfig)
    {
        if (configPath.empty())
        {
            cerr << "Error: Cannot find the default config folder, use --config <file>." << endl;
            return 1;
        }

        try
        {
            writeDefaultConfig(configPath);
        }
        catch (const exception& e)
        {
            cerr << "Error: " << e.what() << endl;
            return 1;
        }

        cout << "Config file created:" << endl;
        cout << "  " << configPath.u8string() << endl;
        cout << "Edit it to change categories, exclusions and destinations." << endl;

        return 0;
    }

    // The undo only needs the log, not the config
    Config config = defaultConfig();

    if (!undo)
    {
        if (!configFile.empty() && !fs::exists(configFile))
        {
            cerr << "Error: The config file " << configFile.u8string()
                 << " doesn't exist" << endl;

            return 1;
        }

        if (!configPath.empty() && fs::exists(configPath))
        {
            try
            {
                config = loadConfig(configPath);
            }
            catch (const ConfigError& e)
            {
                cerr << "Config error: " << e.what() << endl;
                return 1;
            }

            cout << "Using config: " << configPath.u8string() << endl;

            for (const string& warning : config.warnings)
            {
                cerr << "Warning: " << warning << endl;
            }
        }
    }

    // If no directory was provided, ask the user
    if (mainFolder.empty())
    {
        cout << "Insert the path of the main folder: ";

        mainFolder = fs::u8path(readLine());
    }

    if (!fs::exists(mainFolder))
    {
        cerr << "Error: The path is wrong or doesn't exist"
             << endl;

        return 1;
    }

    if (!fs::is_directory(mainFolder))
    {
        cerr << "Error: The specified path is not a folder"
             << endl;

        return 1;
    }

    // Absolute, so the paths in the log don't depend on the current folder
    mainFolder = fs::absolute(mainFolder);

    if (undo && !fs::exists(mainFolder / JOURNAL_FILE_NAME))
    {
        cout << "Nothing to undo: this folder has no operations log." << endl;
        return 0;
    }

    OrganizeSummary summary;

    try
    {
        if (undo)
        {
            summary = undoLastRun(mainFolder, dryRun);
        }
        else
        {
            summary = organizeDirectory(mainFolder, config, dryRun);
        }
    }
    catch (const fs::filesystem_error& e)
    {
        cerr << "Filesystem error: " << e.what() << endl;
        return 1;
    }
    catch (const runtime_error& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    printSummary(summary, dryRun, undo);

    return summary.failed > 0 ? 1 : 0;
}
