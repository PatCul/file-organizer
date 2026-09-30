# 📁 File Organizer

A simple **C++ file organizer** that automatically sorts files into folders based on their file extension.

The project was created as a practical exercise to learn and apply **C++17**, especially the `<filesystem>` library.

## ✨ Features

* 📂 Choose the folder to organize
* 🔍 Automatically detect file extensions
* 🗂️ Categorize files into dedicated folders
* 📦 Move files using `std::filesystem`
* ❓ Move unknown file types into an `Other` folder
* 🚫 Ignore directories and process only regular files
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
g++ -std=c++17 main.cpp -o file-organizer.exe
```

## ▶️ How to Use

Run the program:

```bash
./file-organizer.exe
```

The program will ask you for the path of the folder you want to organize:

```text
Insert the path of the main folder
```

For example:

```text
C:\Users\Patrik\Desktop\Test
```

The program will then scan the files inside the folder and move them into the appropriate category folders.

### Before

```text
Test/
├── foto.jpg
├── documento.pdf
├── canzone.mp3
├── programma.cpp
└── archivio.zip
```

### After

```text
Test/
├── Images/
│   └── foto.jpg
├── Documents/
│   └── documento.pdf
├── Music/
│   └── canzone.mp3
├── Code/
│   └── programma.cpp
└── Archives/
    └── archivio.zip
```

Files with unsupported extensions are moved to:

```text
Other/
```

## 📁 Project Structure

```text
file-organizer/
├── main.cpp
├── README.md
└── .gitignore
```

## 🧠 What I Learned

This project helped me practice several important C++ concepts:

* `std::filesystem::path`
* `std::filesystem::directory_iterator`
* `std::filesystem::directory_entry`
* Checking whether paths and directories exist
* Creating directories programmatically
* Moving files with `std::filesystem::rename`
* Using `std::map` to associate file extensions with categories
* Using iterators and `map::find()`
* Working with C++17 features
* Handling paths and file extensions

## 🔮 Future Improvements

Possible improvements for future versions:

* [ ] Support paths containing spaces
* [ ] Handle uppercase extensions such as `.JPG`
* [ ] Handle files with duplicate names
* [ ] Add better filesystem error handling
* [ ] Add a preview/dry-run mode
* [ ] Add command-line arguments
* [ ] Add recursive folder organization
* [ ] Add configurable categories
* [ ] Display a summary after organizing
* [ ] Separate the project into multiple source/header files

## ⚠️ Warning

This program **moves files automatically**. Always test it on a folder containing copies of your files before using it on important data.

## 📄 License

This project is open source and available under the MIT License.
