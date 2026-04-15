# Academic Transcript Generator

This is a project for the CSE115L course at my university.

## Building the project

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

If `gcc` is not in your PATH, you can run the following command as an alternative:

```bash
<absolute_path_to_mingw>\gcc -Isrc -Iinclude -L. src\atsgen.c -lhpdf -o atsgen.exe
```

While replacing `<absolute_path_to_mingw>` with the right path. If MinGW was installed as
part of Code::Blocks, then it is usually `C:\Program Files\CodeBlocks\MinGW\bin`.

## Licensing

This project is licensed under the [GNU General Public License v3.0 or later](./COPYING).
Copyright (C) Ar Rakin 2026.
