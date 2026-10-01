#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
using namespace std;

namespace fs = filesystem;

int main() {

    //taking the path of the main folder
    cout<<"Insert the path of the main folder"<<endl;

    string path;
    getline(cin, path);
    fs::path mainFolder(path);

    //checking if the path exists and is a folder
    if(!fs::exists(mainFolder)) {
        cerr<<"Error: The path is wrong or don't exists"<<endl;
        return 1;
    }
    if(!fs::is_directory(mainFolder)) {
        cerr<<"Error: The specified path is not a folder"<<endl;
        return 1;
    }

    map<string, string> categories = {

        // Documents
        {".txt", "Documents"},
        {".doc", "Documents"},
        {".docx", "Documents"},
        {".pdf", "Documents"},
        {".odt", "Documents"},
        {".rtf", "Documents"},
        {".tex", "Documents"},

        // Spreadsheets
        {".xls", "Spreadsheets"},
        {".xlsx", "Spreadsheets"},
        {".csv", "Spreadsheets"},
        {".ods", "Spreadsheets"},

        // Presentations
        {".ppt", "Presentations"},
        {".pptx", "Presentations"},
        {".odp", "Presentations"},

        // Images
        {".jpg", "Images"},
        {".jpeg", "Images"},
        {".png", "Images"},
        {".gif", "Images"},
        {".bmp", "Images"},
        {".webp", "Images"},
        {".svg", "Images"},
        {".ico", "Images"},
        {".tiff", "Images"},

        // Videos
        {".mp4", "Videos"},
        {".mkv", "Videos"},
        {".avi", "Videos"},
        {".mov", "Videos"},
        {".wmv", "Videos"},
        {".webm", "Videos"},

        // Audio
        {".mp3", "Music"},
        {".wav", "Music"},
        {".flac", "Music"},
        {".aac", "Music"},
        {".ogg", "Music"},
        {".m4a", "Music"},

        // Archives
        {".zip", "Archives"},
        {".rar", "Archives"},
        {".7z", "Archives"},
        {".tar", "Archives"},
        {".gz", "Archives"},
        {".bz2", "Archives"},

        // Programming
        {".cpp", "Code"},
        {".h", "Code"},
        {".hpp", "Code"},
        {".c", "Code"},
        {".cs", "Code"},
        {".java", "Code"},
        {".py", "Code"},
        {".js", "Code"},
        {".ts", "Code"},
        {".php", "Code"},
        {".html", "Code"},
        {".css", "Code"},
        {".sql", "Code"},
        {".json", "Code"},
        {".xml", "Code"},

        // Executables / installers
        {".exe", "Programs"},
        {".msi", "Programs"},

        // Disk images
        {".iso", "Disk Images"},
        {".img", "Disk Images"},

        // Fonts
        {".ttf", "Fonts"},
        {".otf", "Fonts"},
        {".woff", "Fonts"},
        {".woff2", "Fonts"},

        // E-books
        {".epub", "Ebooks"},
        {".mobi", "Ebooks"},

        // Subtitles
        {".srt", "Subtitles"},
        {".ass", "Subtitles"},

        // 3D / CAD
        {".obj", "3D"},
        {".fbx", "3D"},
        {".stl", "3D"},
        {".blend", "3D"},

        // Database
        {".db", "Database"},
        {".sqlite", "Database"},
        {".sqlite3", "Database"},
        
        // Logs
        {".log", "Logs"},

        //Backups
        {".bak", "Backups"},

        //Temporary
        {".tmp", "Temporary"}


    };

    //reading all the files extensions to understand where to send them
    //fs::directory_iterator(mainFolder);
    for(fs::directory_entry entry : fs::directory_iterator(mainFolder)) {
        if(entry.is_regular_file()) {

            string extension = entry.path().extension().string();
            transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            auto it = categories.find(extension);
            fs::path categoryFolder;

            if (it == categories.end()) {
                // extension not found
                categoryFolder = mainFolder / "Other";
            }
            else {
                // extension found
                categoryFolder = mainFolder / it->second;
            }

            if(!fs::exists(categoryFolder)) {
                fs::create_directory(categoryFolder);
            }

            fs::path newPath = categoryFolder / entry.path().filename();
            fs::rename(entry.path(), newPath);
            
        }
    }

    return 0;
}