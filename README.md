# source-map

[![Version](https://img.shields.io/github/v/tag/uictorius/source-map)](https://github.com/uictorius/source-map/tags)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![CI Build & Lint](https://github.com/uictorius/source-map/actions/workflows/ci.yml/badge.svg)](https://github.com/uictorius/source-map/actions)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](#)

`source-map` is a command-line utility written in C that analyzes a software
project and generates a comprehensive Markdown report. The report includes a
directory tree and the concatenated content of all relevant source files,
respecting `.gitignore` rules.

---

## Features

- **Project Analysis:** Generates a single Markdown file with the project's
  structure and code.
- **Git Aware:** Automatically respects rules from `.gitignore` files.
- **Extensible:** Supports multiple languages via simple, configurable `.ini`
  files.
- **Portable:** Written in standard C with minimal dependencies.
- **Robust:** Includes a native fallback if the `tree` command is not installed.

---

## 🚀 Installation (For Users)

This is the recommended method for most users.

### Prerequisites

- A C Compiler (e.g., GCC or Clang)
- `make`
- `git` (for cloning)
- `tree` (optional, for best directory tree output)

### Install Process

This will clone the repository, compile the source, and install the binary and
default configurations onto your system.

```bash
# 1. Clone the repository
git clone [https://github.com/uictorius/source-map.git](https://github.com/uictorius/source-map.git)
cd source-map

# 2. Compile and install
sudo make install
```

This installs:

- **Binary:** `/usr/local/bin/source-map`
- **Configurations:** `/usr/local/share/source-map/config`

### Uninstallation

```bash
# Must be run from the cloned project directory
sudo make uninstall
```

---

## ⚙️ Usage

Analyze a project from its root directory:

```bash
# Analyze a C project in the current directory
source-map c

# Analyze a Python project in a specific directory
source-map python ./my-python-app

# Specify a different output file
source-map javascript ./my-js-app report.md
```

### Parameters

| Parameter          | Description                                | Default     |
| :----------------- | :----------------------------------------- | :---------- |
| `language_profile` | The language profile (e.g., `c`, `python`) | _Required_  |
| `target_directory` | The project directory to analyze           | `.`         |
| `output_file`      | The name of the output Markdown file       | `output.md` |

---

## 🧩 Language Configuration

Language profiles are defined by `.ini` files. You can create your own and place
them in one of these locations (loaded in order of priority):

1.  `./config` (Inside the project being analyzed)
2.  `~/.config/source-map` (User-specific profiles)
3.  `/usr/local/share/source-map/config` (System-wide defaults)

#### Example (`config/c.ini`):

```ini
[Core]
language_name = C Project Export

[Filters]
; Files allowed by extension
allowed_extensions = c,h,md,ini
; Files allowed by exact filename
allowed_filenames = Makefile,LICENSE,.gitignore,README.md
; Dotfiles allowed
allowed_dotfiles = .gitignore
; Ignored extensions
ignored_extensions = o,d
; Ignored filenames
ignored_filenames = output.md

[Markdown]
; Map extensions or filenames to Markdown syntax tags
syntax_map = c:c,h:c,ini:ini,md:markdown,Makefile:makefile
```

---

## 🛠️ For Developers (Contributing)

Instructions for those who want to build from source to modify or contribute.

### Building for Development

To compile the project without installing it on your system:

```bash
# Make sure prerequisites are installed
make
```

The test executable will be available at `bin/source-map`.

### Code Formatting

This project uses `clang-format` for C and `prettier` for all other files
(`.md`, `.json`, etc.). The configuration files (`.clang-format`,
`.prettierrc.json`) are included.

To format all files automatically:

```bash
# Make sure you have clang-format and npx (npm) installed
make format
```

### Continuous Integration (CI)

All contributions must pass the CI checks (linting and building). The workflow
is defined in `.github/workflows/ci.yml`.

### Project Structure

```
.
├── .github/workflows/  # CI workflows
├── config/             # Default language profiles
├── include/            # Header files
├── src/                # Source code
├── .vscode/            # VS Code settings
├── .clang-format       # C formatting rules
├── .gitignore
├── .prettierrc.json    # Prettier formatting rules
├── LICENSE
├── Makefile
└── README.md
```

---

## 📜 License

Distributed under the **MIT License**. See `LICENSE` for more information.
