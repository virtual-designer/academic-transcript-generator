# Academic Transcript Generator

This is a project for the CSE115L course at my university.

## Building & Running The Project

### GNU/Linux

#### Prerequisites

1. A C compiler, preferably the GNU C Compiler (gcc) version 14 or later
2. GNU Make
3. libharu (for PDF generation)

If libharu is missing, it can be installed in various ways depending on the distribution.
I will assume the user knows how to do that.

To build and run the project, run:

```bash
make run-linux
```

### Darwin (macOS)

#### Prerequisites

1. A C compiler, preferably the Apple Clang C Compiler
2. GNU Make
3. libharu (for PDF generation)

If libharu is missing, it can be installed using [Homebrew](https://brew.sh). Then, if Homebrew is also missing, it can be installed using the following command:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Install libharu using the following command, if not already installed:

```bash
brew install libharu
```

To finally build and run the project, run:

```bash
make run-darwin
```

### Windows

First ensure that you have MinGW/gcc installed and available in your PATH.
Then open a terminal in the project directory and run:

```bash
gcc -Isrc -Iinclude -L. src\atsgen.c -lhpdf -o atsgen.exe
```

Then finally to run the project:

```batch
atsgen.exe
```

Or if using PowerShell:

```powershell
.\atsgen.exe
```

If `gcc` is not in your PATH, either add it to your PATH, or follow the workaround below if using Code::Blocks with MinGW.

If MinGW was installed as part of Code::Blocks, then binaries usually found at `C:\Program Files\CodeBlocks\MinGW\bin`.
The issue is, if that path isn't in your PATH environment variable, the compiler will fail silently without any output file.

To fix it, you have to first open the `src/atsgen.c` file in Code::Blocks, and then edit the compiler flags: Settings > Compiler, then edit any checked compiler option set, and add:

* Compiler flags: `-I<full_project_path>\src -I<full_project_path>\include`
* Linker flags: `-L<full_project_path> -L<full_project_path>\src -lhpdf`

While replacing `<full_project_path>` with the actual absolute path to the project.
Then copy  `libhpdf.dll` from the project root to `src`.

Finally, to build: Build > Build & Run. This should build and run the project.

## Contributors [A-Z]

* Akib Elahi
* Ar Rakin
* Momsad Hossain

## Licensing

This project is licensed under the [GNU General Public License v3.0 or later](./COPYING).
Copyright (C) The Contributors 2026.
