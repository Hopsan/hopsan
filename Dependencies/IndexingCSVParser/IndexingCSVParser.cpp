//!
//! @file   IndexingCSVParser.cpp
//! @author Peter Nordin
//! @date   2015-02-03
//!
//! @brief Contains some of the IndexingCSVParser implementation
//!

#include "IndexingCSVParser.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace indcsvp;

//! @brief Help function that gobbels all characters on a line
//! @param[in] pFile The file object to gobble from
void discardLine(FILE *pFile)
{
    while (fgetc(pFile) != '\n')
    {
        // Run loop till newline has been gobbled
    }
}

IndexingCSVParser::IndexingCSVParser()
{
    mpFile = 0;
    mSeparatorChar = ',';
    mNumSkipLines = 0;
    mCommentChar = '\0';
}

//! @brief Set the separator character
//! @param[in] sep The separator character
void IndexingCSVParser::setSeparatorChar(char sep)
{
    mSeparatorChar = sep;
}

//! @brief Set the comment character
//! @param[in] sep The character to indicate a comment
void IndexingCSVParser::setCommentChar(char com)
{
    mCommentChar = com;
}

//! @brief Set the number of initial lines to ignore
//! @param[in] num The number of lines to ignore
void IndexingCSVParser::setNumLinesToSkip(size_t num)
{
    mNumSkipLines = num;
}

//! @brief Automatically choose the separator character
//! @param[in] rAlternatives A vector with alternatives, the first on encountered will be used.
//! @returns The chosen character, or the previously selected on if no match found
char IndexingCSVParser::autoSetSeparatorChar(const std::vector<char> &rAlternatives)
{
    // Discard header and comments
    readUntilData();

    bool found=false;
    while (!found && !feof(mpFile))
    {
        int c = fgetc(mpFile);
        for (size_t i=0; i<rAlternatives.size(); ++i)
        {
            if (c == rAlternatives[i])
            {
                mSeparatorChar = char(c);
                found = true;
                break;
            }
        }
    }
    return mSeparatorChar;
}

//! @brief Returns the separator character used
char IndexingCSVParser::getSeparatorChar() const
{
    return mSeparatorChar;
}

//! @brief Returns the comment character used
char IndexingCSVParser::getCommentChar() const
{
    return mCommentChar;
}

//! @brief Returns the number of initial lines to skip
size_t IndexingCSVParser::getNumLinesToSkip() const
{
    return mNumSkipLines;
}

//! @brief Open a file in binary read-only mode
//! @param[in] filePath Path to the file to open
//! @returns true if the files was opened successfully else false
bool IndexingCSVParser::openFile(const char *filePath)
{
    mpFile = fopen(filePath, "rb");
    return (mpFile != 0);
}

//! @brief Close the opened file
void IndexingCSVParser::closeFile()
{
    if (mpFile)
    {
        fclose(mpFile);
        mpFile = 0;
    }
    mSeparatorPositions.clear();
}

//! @brief Rewind the file pointer to the beginning of the file
void IndexingCSVParser::rewindFile()
{
    rewind(mpFile);
}

//! @brief Run indexing on the file, to find all separator positions
void IndexingCSVParser::indexFile()
{
    rewindFile();
    readUntilData();
    mSeparatorPositions.clear();

    mSeparatorPositions.reserve(100); //!< @todo guess
    size_t lastLineNumSeparators = 20;

    // We register the position in the file before we read the char, as that will advance the file pointer
    size_t pos = ftell(mpFile);
    int c = fgetc(mpFile);
    while (c!=EOF)
    {
        // Append new line, but we will work with a reference
        mSeparatorPositions.push_back(vector<size_t>());
        vector<size_t> &rLine = mSeparatorPositions.back();
        rLine.reserve(lastLineNumSeparators);

        // Register Start of line position
        rLine.push_back(pos);
        // Now read line until and register each separator char position
        while (c!='\n' && c!='\r' && c!=EOF)
        {
            if (c==mSeparatorChar)
            {
                rLine.push_back(pos);
            }

            // Get next char
            pos = ftell(mpFile);
            c = fgetc(mpFile);
        }
        // Register end of line position
        rLine.push_back(pos);
        // Read pos and first char on next line
        // The while loop make sure we gobble LF if we have CRLF eol
        while ( c == '\r' || c == '\n' )
        {
            pos = ftell(mpFile);
            c = fgetc(mpFile);
        }

        // Remeber the length of the line (to reserve relevant amount of memmory next time)
        lastLineNumSeparators =  rLine.size();
    }
}

//! @brief Returns the number of indexed data rows in the file
//! @note This will only work if the file has been indexed
//! @returns The number of indexed rows
size_t IndexingCSVParser::numRows() const
{
    return mSeparatorPositions.size();
}

//! @brief Returns the number of indexed columns for a particular row
//! @note This will only work if the file has been indexed
//! @param[in] row The row index (0-based)
//! @returns The number of indexed columns on the requested row
size_t IndexingCSVParser::numCols(size_t row) const
{
    if (row < mSeparatorPositions.size())
    {
        return mSeparatorPositions[row].size()-1;
    }
    return 0;
}

//! @brief Check if all indexed rows have the same number of columns
//! @returns true if all rows have the same number of columns, else returns false
bool IndexingCSVParser::allRowsHaveSameNumCols() const
{
    size_t nCols = numCols(0);
    for (size_t r=0; r<mSeparatorPositions.size(); ++r)
    {
        if (numCols(r) != nCols)
        {
            return false;
        }
    }
    return true;
}


void IndexingCSVParser::minMaxNumCols(size_t &rMin, size_t &rMax)
{
    rMin = -1; rMax =0;
    for (size_t r=0; r<mSeparatorPositions.size(); ++r)
    {
        rMin = std::min(rMin, mSeparatorPositions[r].size());
        rMax = std::max(rMax, mSeparatorPositions[r].size());
    }
}

//! @brief Extract the data of a given indexed column (as std::string)
//! @param[in] col The column index (0-based)
//! @param[in,out] rData The data vector to append column data to
//! @returns true if no errors occured, else false
bool IndexingCSVParser::getIndexedColumn(const size_t col, std::vector<string> &rData)
{
    if (col < numCols(0))
    {
        const size_t nr = numRows();
        // Reserve data (will only increase reserved memmory if needed, not shrink)
        rData.reserve(nr);

        CharBuffer cb;

        // Loop through each row
        for (size_t r=0; r<nr; ++r)
        {
            // Begin and end positions
            size_t b = mSeparatorPositions[r][col] + size_t(col > 0);
            size_t e = mSeparatorPositions[r][col+1];
            // Move file ptr
            fseek(mpFile, b, SEEK_SET);

            // Extract data
            cb.resize(e-b+1);
            char* rc = fgets(cb.buff(), e-b+1, mpFile);
            // Push back data
            if (rc)
            {
                rData.push_back(cb.buff());
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

//! @brief Extract the data of a given indexed row (as std::string)
//! @param[in] row The row index (0-based)
//! @param[in,out] rData The data vector to append row data to
//! @returns true if no errors occured, else false
bool IndexingCSVParser::getIndexedRow(const size_t row, std::vector<string> &rData)
{
    if (row < mSeparatorPositions.size())
    {
        const size_t nc = numCols(row);
        // Reserve data (will only increase reserved memmory if needed, not shrink)
        rData.reserve(nc);

        // Begin position
        size_t b = mSeparatorPositions[row][0];
        // Move file ptr
        fseek(mpFile, b, SEEK_SET);
        // Character buffer for extraction
        CharBuffer cb;

        // Loop through each column on row
        for (size_t c=1; c<=nc; ++c)
        {
            const size_t e = mSeparatorPositions[row][c];
            cb.resize(e-b+1);
            char* rc = fgets(cb.buff(), e-b+1, mpFile);
            if (rc)
            {
                rData.push_back(cb.buff());
            }
            else
            {
                return false;
            }

            // Update b for next field, skipping the character itself
            b = mSeparatorPositions[row][c]+1;
            // Move the file ptr, 1 char (gobble the separator)
            fgetc(mpFile);
        }
        return true;
    }
    return false;
}

//! @brief Extract the data of a given indexed position row and column (as std::string)
//! @param[in] row The row index (0-based)
//! @param[in] col The column index (0-based)
//! @returns The value at the requested position as std::string or empty if position does not exist
string IndexingCSVParser::getIndexedPos(const size_t row, const size_t col, bool &rParseOK)
{
    if (row < mSeparatorPositions.size())
    {
        if (col < mSeparatorPositions[row].size())
        {
            // Begin and end positions
            size_t b = mSeparatorPositions[row][col] + size_t(col > 0);
            size_t e = mSeparatorPositions[row][col+1];
            fseek(mpFile, b, SEEK_SET);

            CharBuffer cb(e-b+1);
            char* rc = fgets(cb.buff(), e-b+1, mpFile);
            if (rc)
            {
                rParseOK = true;
                return string(cb.buff());
            }
            else
            {
                rParseOK = false;
            }
        }
    }
    return "";
}

//! @brief Extract a data row from a non-indexed file (as std::string)
//! @param[in,out] rData The data vector to append extracted data to
bool IndexingCSVParser::getRow(std::vector<string> &rData)
{
    bool isSuccess = true;
    CharBuffer cb;

    size_t b = ftell(mpFile);
    while (true)
    {
        size_t e = ftell(mpFile);
        int c = fgetc(mpFile);

        if (c == mSeparatorChar || c == '\n' || c == '\r' || c == EOF)
        {
            // Rewind file pointer to start of field
            fseek(mpFile, b, SEEK_SET);
            cb.resize(e-b+1);
            char* rc = fgets(cb.buff(), e-b+1, mpFile);
            if (rc)
            {
                rData.push_back(cb.buff());
            }
            else
            {
                // Indicate we failed to parse, but we still need to gobble the entire line incase we reach EOF
                isSuccess = false;
            }

            // Eat the separator char, in case of CRLF EOL, then gobble both CR and expected LF
            do
            {
                c = fgetc(mpFile);
                b = ftell(mpFile); //!< @todo maybe can use +1 since binary mode (calc bytes) might be faster
            }while(c == '\r');

            // Break loop when we have reached EOL or EOF
            if (c == '\n' || c == EOF)
            {
                // If we got a LF then peek to see if EOF reached, if so gooble char to set EOF flag on file
                if (peek(mpFile) == EOF)
                {
                    fgetc(mpFile);
                }
                break;
            }
        }
    }
    return isSuccess;
    //! @todo try to index line first before extracting data, might be faster since we can reserve (maybe)
}

//! @brief Check if more data rows are availible for extraction (for non-indexed files)
//! @returns true if more rows are waiting, returns false if filpe pointer has reached EOF
bool IndexingCSVParser::hasMoreDataRows()
{
    return !feof(mpFile);
}

//! @brief Gobble the initial number of lines to skip and lines beginning with the comment character
void IndexingCSVParser::readUntilData()
{
    // First remote lines to skip
    for(size_t i=0; i<mNumSkipLines; ++i)
    {
        discardLine(mpFile);
    }

    // Now remove lines starting with comment sign (if one is set)
    if (mCommentChar != '\0')
    {
        while (peek(mpFile) == mCommentChar)
        {
            discardLine(mpFile);
        }
    }
}
