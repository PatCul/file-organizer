# 📁 File Organizer

A simple **C++ file organizer** that automatically sorts files into folders based on their file extension.

The project was created as a practical exercise to learn and apply **C++17**, especially the `<filesystem>` library.

## ✨ Features

* 📂 Choose the folder to organize (as an argument or interactively)
* 🔍 Automatically detect file extensions (case-insensitive, `.JPG` = `.jpg`)
* 🗂️ Categorize files into dedicated folders
* 📦 Move files using `std::filesystem`
* ❓ Move unknown file types into an `Other` folder
* 🚫 Ignore directories and process only regular files
* 🔁 Never overwrite files: duplicates are renamed (`photo (1).jpg`)
* 👀 Dry-run mode to preview the changes without moving anything
* 🛡️ Per-file error handling: a locked or inaccessible file is reported and skipped, the others are still organized
* 📊 Summary at the end with moved files and errors
* 🧩 Easily extendable with new file extensions and categories

## 📋 Supported Categories

The organizer currently supports categories such as:

| Category      | Examples                                                          |
| ------------- | ----------------------------------------------------------------- |
| Documents     | `.txt`, `.doc`, `.docx`, `.pdf`, `.odt`                           |
| Spreadsheets  | `.xls`, `.xlsx`, `.csv`, `.ods`                                   |
| Presentations | `.ppt`, `.pptx`, `.odp`                                           |
| Images        | `.jpg`, `.jpeg`, `.png`, `.gif`, `.svg`                           |
| Videos        | `.mp4`, `.mkv`, `.avi`, `.mov`, `.webm`                           |
| Music         | `.mp3`, `.wav`, `.flac`, `.aac`, `.m4a`                           |
| Archives      | `.zip`, `.rar`, `.7z`, `.tar`, `.gz`                              |
| Code          | `.cpp`, `.h`, `.hpp`, `.c`, `.java`, `.py`, `.js`, `.php`, `.sql` |
| Programs      | `.exe`, `.msi`                                                    |
| Disk Images   | `.iso`, `.img`                                                    |
| Fonts         | `.ttf`, `.otf`, `.woff`, `.woff2`                                 |
| Ebooks        | `.epub`, `.mobi`                                                  |
| Subtitles     | `.srt`, `.ass`                                                    |
| 3D            | `.obj`, `.fbx`, `.stl`, `.blend`                                  |
| Database      | `.db`, `.sqlite`, `.sqlite3`                                      |
| Logs          | `.log`                                                            |
| Backups       | `.bak`                                                            |
| Temporary     | `.tmp`                                                            |
| Other         | Unknown extensions                                                |

## 🛠️ Technologies

* **C++17**
* `<filesystem>`
* `<iostream>`
* `<string>`
* `<map>`

No external libraries are required.

## 🚀 How to Build

Make sure you have a C++ compiler with **C++17** support installed.

For example, using `g++`:

```bash
g++ -std=c++17 src/main.cpp src/organizer.cpp src/utils.cpp -o file-organizer.exe
```

## ▶️ How to Use

Run the program:

```text
file-organizer [options] [directory]
```

| Option      | Description                                       |
| ----------- | ------------------------------------------------- |
| `--dry-run` | Show what would be moved, without moving any file |
| `--help`    | Show the help message                             |

Examples:

```bash
./file-organizer.exe "C:\Users\Username\Desktop\Test"
```

```bash
./file-organizer.exe --dry-run "C:\Users\Username\Desktop\Test"
```

If no directory is given, the program asks for it:

```text
Insert the path of the main folder: C:\Users\Username\Desktop\Test
```

The program scans the files inside the folder and moves them into the appropriate category folders. At the end it prints a summary:

```text
Summary
  Files moved: 5
  Errors: 0
```

If some files could not be moved (for example because they are open in another program), they are listed under the summary and the program exits with code `1`.

### Before

```text
Test/
├── image.jpg
├── document.pdf
├── song.mp3
├── program.cpp
└── archive.zip
```

### After

```text
Test/
├── Images/
│   └── image.jpg
├── Documents/
│   └── document.pdf
├── Music/
│   └── song.mp3
├── Code/
│   └── program.cpp
└── Archives/
    └── archive.zip
```

Files with unsupported extensions are moved to:

```text
Other/
```

## 📁 Project Structure

```text
file-organizer/
├── src/
│   ├── main.cpp          # Command-line parsing and summary output
│   ├── organizer.cpp     # Categories and file moving logic
│   ├── organizer.hpp
│   ├── utils.cpp         # Helper functions (toLower)
│   └── utils.hpp
├── LICENSE
├── README.md
└── .gitignore
```

## 🔮 Future Improvements

Possible improvements for future versions:

* [x] Support paths containing spaces
* [x] Handle uppercase extensions such as `.JPG`
* [x] Handle files with duplicate names
* [x] Add better filesystem error handling
* [x] Add a preview/dry-run mode
* [x] Add command-line arguments
* [x] Display a summary after organizing
* [x] Separate the project into multiple source/header files
* [ ] Undo the last organization (operation log)
* [ ] Add configurable categories (external config file)
* [ ] Ignore incomplete downloads (`.crdownload`, `.part`)
* [ ] Watch mode to organize new files automatically
* [ ] Add recursive folder organization

## ⚠️ Warning

This program **moves files automatically**. Use `--dry-run` first, and test it on a folder containing copies of your files before using it on important data.

## 📄 License

This project is open source and available under the MIT License.
