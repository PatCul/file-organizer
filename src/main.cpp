#include <iostream>
#include <string>
#include <filesystem>

#include "organizer.hpp"

using namespace std;

namespace fs = filesystem;

int main()
{
    // Taking the path of the main folder
    cout << "Insert the path of the main folder: ";

    string path;
    getline(cin, path);

    fs::path mainFolder(path);

    // Checking if the path exists
    if (!fs::exists(mainFolder))
    {
        cerr << "Error: The path is wrong or doesn't exist"
             << endl;

        return 1;
    }

    // Checking if the path is a folder
    if (!fs::is_directory(mainFolder))
    {
        cerr << "Error: The specified path is not a folder"
             << endl;

        return 1;
    }

    organizeDirectory(mainFolder);

    return 0;
}