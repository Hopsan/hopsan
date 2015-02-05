# IndexingCSVParser

## Introduction

This is a simple CSV (Comma Separated Value) ASCII text file parser implemented as a C++ class.
It can operate in two modes.

1. Parse all data in the input file line by line.
2. Index the file to make it possible to extract and parse individual rows or columns

Method 1 is suitable if you want to parse all data in the file.<br>
Method 2 is suitable if you have a large data sets and only want to extract (and parse) one or a few specific columns or rows.

## Features
- Cross platform support, supports Windows, Linux and Mac.
- Supports both LF and CRLF end-of-line characters.
- You can chose what separator character to use.
- You can specify a "comment" character, to make the parser ignore initial lines begining with that character.
- You can specify the number of initial lines to skip.
- Or the parser can automatically detect which separator character to use (based on supplied alternatives)
- Indexing allows you to count the number of rows and columns before extracting data. This allows you to reserve memory before extraction and parsing begins.
- You can extract (and parse) individual rows or column (if file has been indexed).
- Template functions allow you to extract data and parse it as a specific type directly, insted of first extracting as string and then converting in an additional step.

## Current Limitations
- Error handling is currently absent, it is assumed that no read errors on the file occur.
- Input files are required to have a new-line at the end of the data, (blank empty line at the end)
- The data file must be "well formed" the same seprator charter must be used in the entire file 
- The parser does not yet support "fields". A way to enclose text with the separator character, withoout it being interpreted as a seprator.
- No automated testing (and more test examples are needed)
- Supports only CRLF or LF eol characters, (but this is enough for my own needs)
- Only supports ASCII input (no unicode support)

## License
This code is released under the Simplifed BSD license. See the LICENSE file for details

## Build
The library is written in "plain C++", the project file (.pro) is a QtCreator project file but you do not need Qt to build the library. Just include the header and cpp files in your own project and compile it.

## Usage Example 
See the main file for usage example, and read the doxygen comments in the code (cpp file) 

