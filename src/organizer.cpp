#include "organizer.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "utils.hpp"

OrganizeSummary organizeDirectory(const fs::path& directory, bool dryRun)
{
    std::map<std::string, std::string> categories = {

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

        // Backups
        {".bak", "Backups"},

        // Temporary
        {".tmp", "Temporary"}
    };

    // Collect the files first: moving them while iterating the
    // directory would modify it during the iteration
    std::vector<fs::path> files;

    for (const fs::directory_entry& entry : fs::directory_iterator(directory))
    {
        if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }

    OrganizeSummary summary;

    for (const fs::path& file : files)
    {
        // A single failing file (locked, missing permissions...)
        // must not stop the organization of the others
        try
        {
            std::string extension = toLower(file.extension().string());

            auto it = categories.find(extension);

            fs::path categoryFolder;

            if (it == categories.end())
            {
                categoryFolder = directory / "Other";
            }
            else
            {
                categoryFolder = directory / it->second;
            }

            if (!fs::exists(categoryFolder))
            {
                if (!dryRun)
                {
                    fs::create_directory(categoryFolder);
                }
            }

            fs::path newPath = categoryFolder / file.filename();

            newPath = getUniquePath(newPath);

            if (!dryRun)
            {
                fs::rename(file, newPath);
            }
            else
            {
                std::cout << "[DRY RUN] "
                    << file
                    << " -> "
                    << newPath
                    << std::endl;
            }

            summary.moved++;
        }
        catch (const fs::filesystem_error& e)
        {
            summary.failed++;
            summary.errors.push_back(file.filename().string() + ": " + e.what());
        }
    }

    return summary;
}

fs::path getUniquePath(const fs::path& path)
{
    if (!fs::exists(path))
    {
        return path;
    }

    int counter = 1;

    fs::path newPath;

    do
    {
        std::string name =
            path.stem().string()
            + " ("
            + std::to_string(counter)
            + ")"
            + path.extension().string();

        newPath = path.parent_path() / name;

        counter++;

    } while (fs::exists(newPath));

    return newPath;
}