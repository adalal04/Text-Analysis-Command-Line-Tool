# MADCounter - Madison Text Analysis Utility

## Overview

MADCounter is a command-line utility designed to perform detailed text analysis, providing statistics on character and word frequencies, unique character counts, line counts, and more from the given text content. Developed in C, this tool aims to help users re-familiarize themselves with programming concepts such as file I/O, string manipulation, dynamic memory allocation, and the Linux environment.

## Objectives

- Enhance familiarity with the C programming language and the Linux programming environment.
- Learn about file handling, string operations, and dynamic memory management in C.
- Gain practical experience with developing Unix command-line utilities.

## Features

MADCounter analyzes text files to report:
- Total and unique character counts, along with their frequencies.
- Word frequency analysis, identifying total and unique words and their initial positions.
- Line analysis to count total and unique lines and their initial appearances.
- Identification of the longest word(s) and line(s) in the text, including their lengths.

### Command Flags
- f <input file>: Specifies the input file to analyze.
- o <output file>: Directs the output to a specified file. If not used, output is printed to stdout.
- c: Enables character analysis.
- w: Enables word frequency analysis.
- l: Enables line analysis.
- Lw: Finds and reports the longest word(s) in the text.
- Ll: Finds and reports the longest line(s) in the text.
- B <batch file>: Executes commands from a specified batch file in batch mode.

## Execution Modes

### Interactive Mode

Run `./MADCounter` followed by specific flags and arguments to analyze a single file directly from the command line.

Example:

```bash
./MADCounter -f input.txt -o output.txt -c -w -l
```


