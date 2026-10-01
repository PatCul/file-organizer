# File Organizer

A lightweight command-line file organizer written in C++17.

File Organizer automatically organizes files in a directory into categorized subdirectories based on their file extensions. It also provides a safe **dry-run mode**, duplicate filename handling, command-line arguments, and filesystem error handling.

## Features

* 📁 Organizes files into categories based on their extensions
* 🔍 Supports a large number of common file formats
* 🛡️ Safe duplicate filename handling
* 👀 `--dry-run` mode to preview operations without modifying files
* 💻 Command-line interface
* ❓ `--help` command
* ⚠️ Filesystem error handling
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
| Temporary     | `.tmp`                                                                                    |

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
g++ -std=c++17 src/main.cpp src/organizer.cpp src/utils.cpp -o file-organizer
```

On Windows:

```bash
g++ -std=c++17 src/main.cpp src/organizer.cpp src/utils.cpp -o file-organizer.exe
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
│   ├── utils.cpp
│   └── utils.hpp
├── CMakeLists.txt
├── .gitignore
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
* Dry-run processing
* File moving

### `organizer.hpp`

Contains declarations related to the file organization functionality.

### `utils.cpp`

Contains utility functions used by the project, such as case conversion for file extensions.

### `utils.hpp`

Contains declarations for utility functions.

## Error Handling

The program validates the specified path before attempting to organize it.

It checks whether:

* The path exists
* The path is a directory
* Filesystem operations succeed

Filesystem exceptions are handled using `std::filesystem::filesystem_error`.

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
* Operation statistics
* Logging
* Undo functionality
* Configuration file for custom categories
* User-defined extension mappings
* Hash-based duplicate detection
* Automatic/watch mode
* More advanced command-line options

## License

This project is provided for educational and personal use.
