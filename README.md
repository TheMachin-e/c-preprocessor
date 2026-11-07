# C Preprocessor

A C preprocessor implemented from scratch in C.

The goal of this project is to understand how the C preprocessing stage works internally by implementing its core functionality without relying on an existing preprocessing library.

The project focuses on the commonly used preprocessing features, including header file inclusion, macro definitions and replacement, conditional compilation, and preprocessing directives.

## Features

### Implemented

- [x] Read source files into memory.
- [x] Handle line splicing (`\` followed by a newline).
- [x] Lexical analysis and tokenization.
- [x] Remove single-line and multi-line comments.
- [x] Handle recursive header file inclusion using `#include`.
- [x] Maintain a macro table using a hash table.
- [x] Define and undefine macros using `#define` and `#undef`.
- [x] Perform object-like macro replacement.
- [x] Support recursive macro expansion.
- [x] Conditional compilation using:
  - `#ifdef`
  - `#ifndef`
  - `#else`
  - `#endif`
- [x] Skip inactive conditional compilation sections.

### Planned

- [ ] Function-like macros.
- [ ] Macro argument parsing and substitution.
- [ ] Stringification operator (`#`).
- [ ] Token-pasting operator (`##`).
- [ ] Expression-based conditional compilation:
  - `#if`
  - `#elif`
- [ ] Predefined macros:
  - `__FILE__`
  - `__LINE__`
  - `__DATE__`
  - `__TIME__`
- [ ] Additional preprocessing directives:
  - `#error`
  - `#warning`
  - `#line`
  - `#pragma`

## Project Structure

```text
.
├── include/
│   └── lexer.h
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── preprocessor.c
│   ├── macro_replacement.c
│   ├── conditional_compiling.c
│   └── utilities.c
├── tests/
├── Makefile
└── README.md
```

### Directory Overview

- **include/** – Header files and shared declarations.
- **src/** – Source files containing the lexer, preprocessor, macro replacement, conditional compilation, and utility implementations.
- **tests/** – Test cases for verifying preprocessing functionality.
- **Makefile** – Build configuration.

## Building

Clone the repository:

```bash
git clone <repository-url>
cd <repository-name>
```

Compile the project using Make:

```bash
make
```

To remove generated object files and the executable:

```bash
make clean
```

## Usage

Run the generated executable with a C source file as input and specify the output file according to the program's command-line interface.

For example:

```bash
./mypreprocessor input.c output.i
```

The output contains the preprocessed source code.

## How It Works

The preprocessor follows a sequence of operations to transform the input source file.

```text
       Input C Source File
                |
                v
          Read File
                |
                v
          Line Splicing
                |
                v
             Lexer
                |
                v
         Token Generation
                |
                v
          Preprocessor
                |
        +-------+-------+
        |       |       |
        v       v       v
     Include  Macros  Conditional
     Handling        Compilation
        |       |       |
        +-------+-------+
                |
                v
       Preprocessed Output
```

### Header File Inclusion

When an `#include` directive is encountered, the specified header file is located and processed recursively.

The included file goes through preprocessing as well, and its resulting output is incorporated into the parent source file.

### Macro Replacement

Macro definitions are stored in a hash table.

When an identifier matching a defined macro is encountered, its replacement tokens are expanded. Recursive expansion is supported for object-like macros.

### Conditional Compilation

Conditional directives determine whether a section of source code should be included in the output.

A conditional stack is maintained to handle nested conditional compilation blocks.

## Testing

The `tests/` directory contains test cases for verifying the implemented functionality.

Testing focuses on:

- Lexer and token recognition.
- Comment removal.
- Header file inclusion.
- Macro definition and replacement.
- Recursive macro expansion.
- Conditional compilation.
- Nested conditional blocks.
- Invalid preprocessing directives and error handling.

The output can also be compared with GCC's preprocessing output to identify differences in behavior.

For reference, GCC can preprocess a file using:

```bash
gcc -E input.c -o gcc_output.i
```

## Project Scope

This project aims to implement a practical subset of the C preprocessing functionality.

It is not intended to be a fully standards-compliant replacement for GCC or Clang. The focus is on understanding preprocessing concepts and implementing commonly used features incrementally.

## Technologies

- **Language:** C
- **Compiler:** GCC
- **Build System:** GNU Make
- **Concepts:** Lexical Analysis, Tokenization, Hash Tables, Dynamic Memory Allocation, Recursion, Macro Expansion, Conditional Compilation using stack

## Future Improvements

The project will be extended incrementally by implementing function-like macros, macro operators, expression-based conditional compilation, and additional preprocessing directives.

