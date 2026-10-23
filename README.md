# File Organizer

A lightweight command-line file organizer written in C++17.

File Organizer automatically organizes files in a directory into categorized subdirectories based on their file extensions. It also provides a safe **dry-run mode**, an **undo** command, duplicate filename handling, command-line arguments, and per-file error handling.

## Features

* 📁 Organizes files into categories based on their extensions
* 🔍 Supports a large number of common file formats
* 🛡️ Safe duplicate filename handling
* 👀 `--dry-run` mode to preview operations without modifying files
* 💻 Command-line interface
* ❓ `--help` command
* ⚠️ Per-file error handling: a locked or inaccessible file is reported and skipped, the others are still organized
* ⏳ Incomplete downloads (`.crdownload`, `.part`, `.download`, `.tmp`) are left untouched
* ↩️ `--undo` command to restore the last organization
* 📊 Summary at the end with moved, skipped and failed files
* 📦 Files with unknown extensions are moved to `Other`
* 🔤 Case-insensitive file extension handling
* 🧩 Modular C++ source structure

## Supported Categories

The organizer currently supports categories such as:

| Category      | Examples                                                                                  |
| ------------- | ----------------------------------------------------------------------------------------- |
| Documents     | `.txt`, `.pdf`, `.docx`, `.odt`, `.rtf`, `.tex`                                           |
| Spreadsheets  | `.xls`, `.xlsx`, `.csv`, `.ods`                                                           |
| Presentations | `.ppt`, `.pptx`, `.odp`                                                                   |
| Images        | `.jpg`, `.jpeg`, `.png`, `.gif`, `.bmp`, `.webp`, `.svg`                                  |
| Videos        | `.mp4`, `.mkv`, `.avi`, `.mov`, `.wmv`, `.webm`                                           |
| Music         | `.mp3`, `.wav`, `.flac`, `.aac`, `.ogg`, `.m4a`                                           |
| Archives      | `.zip`, `.rar`, `.7z`, `.tar`, `.gz`, `.bz2`                                              |
| Code          | `.cpp`, `.c`, `.h`, `.hpp`, `.java`, `.py`, `.js`, `.ts`, `.php`, `.html`, `.css`, `.sql` |
| Programs      | `.exe`, `.msi`                                                                            |
| Disk Images   | `.iso`, `.img`                                                                            |
| Fonts         | `.ttf`, `.otf`, `.woff`, `.woff2`                                                         |
| Ebooks        | `.epub`, `.mobi`                                                                          |
| Subtitles     | `.srt`, `.ass`                                                                            |
| 3D            | `.obj`, `.fbx`, `.stl`, `.blend`                                                          |
| Database      | `.db`, `.sqlite`, `.sqlite3`                                                              |
| Logs          | `.log`                                                                                    |
| Backups       | `.bak`                                                                                    |

Files whose extensions are not recognized are placed in the `Other` directory.

## How It Works

Given a directory such as:

```text
example-directory/
├── photo.jpg
├── document.pdf
├── video.mp4
├── program.cpp
└── archive.zip
```

File Organizer transforms it into:

```text
example-directory/
├── Images/
│   └── photo.jpg
├── Documents/
│   └── document.pdf
├── Videos/
│   └── video.mp4
├── Code/
│   └── program.cpp
└── Archives/
    └── archive.zip
```

The original files are moved into the appropriate category directories.

At the end, the program prints a summary:

```text
Summary
  Files moved: 5
  Skipped (incomplete downloads): 1
  Errors: 0
```

If some files could not be moved (for example because they are open in another program), they are listed under the summary, the other files are still organized, and the program exits with code `1`.

## Duplicate Files

File Organizer never overwrites an existing file.

If a destination already contains:

```text
photo.jpg
```

the organizer generates:

```text
photo (1).jpg
```

If that file also exists:

```text
photo (2).jpg
```

and so on.

This ensures that existing files are preserved.

## Dry Run

The `--dry-run` option allows you to preview what the organizer would do without actually moving or modifying any files.

```bash
file-organizer --dry-run
```

You can also specify the directory directly:

```bash
file-organizer --dry-run "C:\path\to\directory"
```

Example output:

```text
[DRY RUN] "C:\path\to\directory\photo.jpg" -> "C:\path\to\directory\Images\photo.jpg"
[DRY RUN] "C:\path\to\directory\document.pdf" -> "C:\path\to\directory\Documents\document.pdf"
```

No files are moved while running in dry-run mode.

## Incomplete Downloads

Files that are still being downloaded are never moved, because moving them would break the download:

| Extension     | Created by    |
| ------------- | ------------- |
| `.crdownload` | Chrome, Edge  |
| `.part`       | Firefox       |
| `.download`   | Safari        |
| `.tmp`        | Many programs |

They are counted as skipped in the summary and will be organized in a later run, once the download is complete.

## Undo

Every organization is recorded in a log file, `.file-organizer.log`, inside the organized directory. The `--undo` option uses it to move every file back to its original place and to remove the category folders created by the organizer (only if they are empty):

```bash
file-organizer --undo "C:\path\to\directory"
```

It can be combined with `--dry-run` to preview what would be restored:

```bash
file-organizer --undo --dry-run "C:\path\to\directory"
```

Things to know:

* Only the **last** organization can be undone: each run that moves at least one file replaces the log.
* If a file can't be restored (for example because a file with the same name was created in the meantime), it is reported and kept in the log, so you can fix the problem and run `--undo` again.
* When everything has been restored, the log file is deleted.

## Command-Line Usage

### Interactive mode

Run the program without arguments:

```bash
file-organizer
```

The program will ask for the directory to organize.

### Specify a directory

```bash
file-organizer "C:\path\to\directory"
```

### Dry run

```bash
file-organizer --dry-run
```

The program will ask for the directory and simulate the organization.

### Dry run with a directory

```bash
file-organizer --dry-run "C:\path\to\directory"
```

### Undo the last organization

```bash
file-organizer --undo "C:\path\to\directory"
```

### Help

```bash
file-organizer --help
```

This displays the available options and usage examples.

## Building

### Requirements

* C++17 compatible compiler
* CMake 3.15+ or a compiler supporting C++17
* Standard C++ library with `<filesystem>` support

### Compile with g++

```bash
g++ -std=c++17 src/main.cpp src/organizer.cpp src/journal.cpp src/utils.cpp -o file-organizer
```

On Windows:

```bash
g++ -std=c++17 src/main.cpp src/organizer.cpp src/journal.cpp src/utils.cpp -o file-organizer.exe
```

### Run

Linux/macOS:

```bash
./file-organizer
```

Windows:

```powershell
.\file-organizer.exe
```

## Project Structure

```text
file-organizer/
├── src/
│   ├── main.cpp
│   ├── organizer.cpp
│   ├── organizer.hpp
│   ├── journal.cpp
│   ├── journal.hpp
│   ├── utils.cpp
│   └── utils.hpp
├── .gitignore
├── LICENSE
└── README.md
```

### `main.cpp`

Handles the command-line interface, user input, argument parsing, validation, and error handling.

### `organizer.cpp`

Contains the main organization logic, including:

* File extension detection
* Category selection
* Directory creation
* Duplicate filename handling
* Skipping incomplete downloads
* Dry-run processing
* File moving
* Undo of the last organization

### `organizer.hpp`

Contains declarations related to the file organization functionality.

### `journal.cpp`

Reads and writes the operations log (`.file-organizer.log`) used by the undo command. Each line records one operation, with tab-separated fields:

```text
MKDIR	Documents
MOVE	report.pdf	Documents/report.pdf
```

### `journal.hpp`

Contains the declarations of the operations log.

### `utils.cpp`

Contains utility functions used by the project, such as case conversion for file extensions and UTF-8 handling of command-line arguments and console input on Windows, so paths with accented characters (for example `C:\Users\Username\Università`) work correctly.

### `utils.hpp`

Contains declarations for utility functions.

## Error Handling

The program validates the specified path before attempting to organize it.

It checks whether:

* The path exists
* The path is a directory
* Filesystem operations succeed

Errors are handled file by file: if a file can't be moved, the error is recorded and the program continues with the next file. All errors are listed in the summary at the end, and the program exits with code `1`.

## Design

The project is intentionally built using the C++ standard library without external dependencies.

Some of the main standard library features used include:

* `std::filesystem`
* `std::map`
* `std::string`
* `std::transform`
* `std::tolower`
* `std::to_string`

## Future Improvements

Possible future improvements include:

* Recursive directory organization
* File organization by date
* File organization by size
* Configuration file for custom categories
* User-defined extension mappings
* Hash-based duplicate detection
* Automatic/watch mode
* More advanced command-line options

## License

This project is provided for educational and personal use.
