# File Organizer

A lightweight command-line file organizer written in C++17.

File Organizer automatically organizes files in a directory into categorized subdirectories based on their file extensions. It also provides a safe **dry-run mode**, an **undo** command, a **config file** for custom categories and destinations, duplicate filename handling, command-line arguments, and per-file error handling.

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
* ⚙️ Config file (TOML) for categories, exclusions and destinations, also outside the organized folder
* 📦 Files with unknown extensions are moved to `Other`
* 🔤 Case-insensitive file extension handling
* 🧩 Modular C++ source structure

## Supported Categories

Without a config file, the organizer uses these default categories (they can be changed, see [Configuration](#configuration)):

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

The Windows system files `desktop.ini` and `Thumbs.db` are never moved.

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
  Skipped: 3
    incomplete downloads: 1
    excluded: 2
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
[DRY RUN] photo.jpg -> Images\photo.jpg
[DRY RUN] document.pdf -> Documents\document.pdf
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

They are counted as skipped in the summary and will be organized in a later run, once the download is complete. This can't be changed by the config file, so a wrong config can't break a download.

## Configuration

Categories, exclusions and destinations can be changed with a [TOML](https://toml.io) config file, without recompiling.

Create a config file containing the default categories, then edit it:

```bash
file-organizer --init-config
```

The file is created in the default location:

| System        | Default config file                                              |
| ------------- | ---------------------------------------------------------------- |
| Windows       | `%APPDATA%\file-organizer\config.toml`                           |
| Linux / macOS | `~/.config/file-organizer/config.toml`                           |

If that file exists, it is used automatically. Otherwise the [default categories](#supported-categories) are used. A different file can be used with `--config`:

```bash
file-organizer --config "C:\path\to\config.toml" "C:\path\to\directory"
```

`--init-config --config "C:\path\to\config.toml"` creates the file in that location instead.

### Example

```toml
# Folder for files that match no rule.
# Empty text ("") = leave them where they are
other = "Other"

# Files that are never moved: * = any characters, ? = one character
exclude = ["desktop.ini", "thumbs.db", "*.lnk", "keep-*"]

# Rules are checked in order: the first rule with the extension wins
[[rule]]
folder = "Images"
extensions = [".jpg", ".jpeg", ".png"]

[[rule]]
folder = "Work/Invoices"
extensions = [".pdf"]

[[rule]]
# Absolute path: files are moved outside the organized folder
folder = "C:/Users/Username/Music"
extensions = [".mp3", ".flac"]
```

### Keys

| Key            | Description                                                                                       |
| -------------- | ------------------------------------------------------------------------------------------------- |
| `other`        | Folder for files that match no rule. `""` leaves them in place. Default: `"Other"`                 |
| `exclude`      | File name patterns that are never moved (case-insensitive, `*` and `?` allowed)                    |
| `[[rule]]`     | A category. If the file has no `[[rule]]` sections, the default categories are used                |
| `folder`       | Destination: relative to the organized folder (`"Images"`, `"Work/Invoices"`) or a complete path  |
| `extensions`   | List of extensions with the dot (`".pdf"`), case-insensitive                                       |

Notes:

* In paths use `/`, `\\` or single quotes: `"C:/Music"`, `"C:\\Music"` and `'C:\Music'` are the same folder.
* Folders that don't exist are created (also nested ones like `"Work/Invoices"`), and removed by `--undo` if they are empty.
* Files moved to another drive (for example from `C:` to `D:`) are copied and then deleted from the original place.
* Network folders (`\\server\share`) are not supported yet.
* If an extension appears in two rules, the first one wins and a warning is shown.
* The config is checked before moving any file: unknown keys, extensions without the dot or invalid folders stop the program with the line of the error:

```text
Config error: config.toml:12: unknown key 'extension' in [[rule]] (allowed: folder, extensions)
```

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

### Use a config file

```bash
file-organizer --config "C:\path\to\config.toml" "C:\path\to\directory"
```

### Create the default config file

```bash
file-organizer --init-config
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
g++ -std=c++17 src/*.cpp -o file-organizer
```

On Windows:

```bash
g++ -std=c++17 src/*.cpp -o file-organizer.exe
```

The only dependency, [toml++](https://github.com/marzer/tomlplusplus), is included in `third_party/`, so nothing needs to be installed.

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
│   ├── config.cpp
│   ├── config.hpp
│   ├── organizer.cpp
│   ├── organizer.hpp
│   ├── journal.cpp
│   ├── journal.hpp
│   ├── utils.cpp
│   └── utils.hpp
├── third_party/
│   └── toml.hpp      # toml++ 3.4.0 (MIT license)
├── .gitignore
├── LICENSE
└── README.md
```

### `main.cpp`

Handles the command-line interface, user input, argument parsing, config loading, validation, and error handling.

### `config.cpp`

Contains the default categories and loads the config file, checking every value and reporting errors with their line number. It also creates the default config file for `--init-config`.

### `config.hpp`

Contains the declarations of the configuration (`Config`, `Rule`).

### `organizer.cpp`

Contains the main organization logic, including:

* File extension detection
* Rule selection from the config
* Exclusions and skipping incomplete downloads
* Directory creation
* Duplicate filename handling
* Dry-run processing
* File moving, also between different drives
* Undo of the last organization

### `organizer.hpp`

Contains declarations related to the file organization functionality.

### `journal.cpp`

Reads and writes the operations log (`.file-organizer.log`) used by the undo command. Each line records one operation, with tab-separated fields:

```text
MKDIR	Documents
MOVE	report.pdf	Documents/report.pdf
MOVE	song.mp3	C:/Users/Username/Music/song.mp3
```

Paths inside the organized folder are relative, so the folder can be renamed or moved before an undo; destinations outside it are absolute.

### `journal.hpp`

Contains the declarations of the operations log.

### `utils.cpp`

Contains utility functions used by the project, such as case conversion for file extensions, wildcard matching for exclusions, and UTF-8 handling of command-line arguments and console input on Windows, so paths with accented characters (for example `C:\Users\Username\Università`) work correctly.

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

The project is built using the C++ standard library. The only external library is [toml++](https://github.com/marzer/tomlplusplus), a single header used to read the config file, included in `third_party/`.

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
* Rules based on file name and age
* Network folders as destinations
* Hash-based duplicate detection
* Automatic/watch mode
* More advanced command-line options

## License

This project is provided for educational and personal use.
