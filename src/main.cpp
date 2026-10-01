#include <iostream>
#include <string>
#include <filesystem>

#include "organizer.hpp"

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
    cout << "  --dry-run    Simulate the organization without moving files" << endl;
    cout << "  --help       Show this help message" << endl;
    cout << endl;

    cout << "Examples:" << endl;
    cout << "  file-organizer" << endl;
    cout << "  file-organizer --dry-run" << endl;
    cout << "  file-organizer \"C:\\path\\to\\directory\"" << endl;
    cout << "  file-organizer --dry-run \"C:\\path\\to\\directory\"" << endl;
}

int main(int argc, char* argv[])
{
    bool dryRun = false;
    fs::path mainFolder;

    for (int i = 1; i < argc; i++)
    {
        string argument = argv[i];

        if (argument == "--dry-run")
        {
            dryRun = true;
        }
        else if (argument == "--help")
        {
            printHelp();
            return 0;
        }
        else if (mainFolder.empty())
        {
            mainFolder = argument;
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

        string path;
        getline(cin, path);

        mainFolder = path;
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

    try
    {
        organizeDirectory(mainFolder, dryRun);
    }
    catch (const fs::filesystem_error& e)
    {
        cerr << "Filesystem error: " << e.what() << endl;
        return 1;
    }

    return 0;
}