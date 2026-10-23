#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

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
    cout << "  --dry-run    Simulate the operation without moving files" << endl;
    cout << "  --undo       Undo the last organization of the directory" << endl;
    cout << "  --help       Show this help message" << endl;
    cout << endl;

    cout << "Examples:" << endl;
    cout << "  file-organizer" << endl;
    cout << "  file-organizer --dry-run" << endl;
    cout << "  file-organizer \"C:\\path\\to\\directory\"" << endl;
    cout << "  file-organizer --dry-run \"C:\\path\\to\\directory\"" << endl;
    cout << "  file-organizer --undo \"C:\\path\\to\\directory\"" << endl;
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
        cout << "  Skipped (incomplete downloads): " << summary.skipped << endl;
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
    fs::path mainFolder;

    for (const string& argument : getArguments(argc, argv))
    {
        if (argument == "--dry-run")
        {
            dryRun = true;
        }
        else if (argument == "--undo")
        {
            undo = true;
        }
        else if (argument == "--help")
        {
            printHelp();
            return 0;
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
            summary = organizeDirectory(mainFolder, dryRun);
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
