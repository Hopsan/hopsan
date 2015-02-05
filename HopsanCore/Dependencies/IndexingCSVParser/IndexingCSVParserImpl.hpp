//!
//! @file   IndexingCSVParserImpl.hpp
//! @author Peter Nordin
//! @date   2015-02-03
//!
//! @brief Contains the header only (template) implementation of the IndexingCSVParser and utility template functions
//!

#ifndef INDEXINGCSVPARSERIMPL_HPP
#define INDEXINGCSVPARSERIMPL_HPP

#include <cstdio>
#include <cstdlib>
#include <string>

namespace indcsvp
{

//! @brief The default converter template function
//! @details This function will allways fail, template speciallisations for each type are required
//! @tparam T The type that we want to interpret the contests of pBuffer as.
//! @param[in] pBuff The character buffer to convert
//! @param[out] rIsOK Reference to bool flag tellig you if parsing completed successfully
//! @returns Type default constructed value;
template <typename T>
T converter(const char* pBuff, bool &rIsOK)
{
    rIsOK = false;
    return T();
}

//! @brief The std::string converter speciallized template function
//! @param[in] pBuff The character buffer to convert
//! @param[out] rIsOK Reference to bool flag telling you if parsing completed successfully
//! @returns The contents of pBuff as a std::string
template<> inline
std::string converter<std::string>( const char* pBuff, bool &rIsOK)
{
    rIsOK = true;
    return std::string(pBuff);
}

template<> inline
double converter<double>( const char* pBuff, bool &rIsOK)
{
    char *pEnd;
    double d = std::strtod(pBuff, &pEnd);
    rIsOK = (*pEnd == '\0');
    return d;
}

template<> inline
unsigned long int converter<unsigned long int>( const char* pBuff, bool &rIsOK)
{
    char *pEnd;
    long int ul = strtoul(pBuff, &pEnd, 10); //!< @todo maybe support other bases then 10, see strtol documentation
    rIsOK = (*pEnd == '\0');
    return ul;
}

template<> inline
long int converter<long int>( const char* pBuff, bool &rIsOK)
{
    char *pEnd;
    long int i = strtol(pBuff, &pEnd, 10); //!< @todo maybe support other bases then 10, see strtol documentation
    rIsOK = (*pEnd == '\0');
    return i;
}


//! @brief Peek help function to peek at the next character in the file
//! @param[in] pStream The stream too peek in
//! @returns The next character (as int)
inline int peek(FILE *pStream)
{
    int c = fgetc(pStream);
    ungetc(c, pStream);
    return c;
}

//! @brief Character buffer help class, with automatic memmory dealocation and smart reallocation
class CharBuffer
{
public:
    CharBuffer() : mpCharArray(0), mSize(0) {}
    CharBuffer(size_t size) : mpCharArray(0), mSize(0) {resize(size);}
    ~CharBuffer()
    {
        if (mpCharArray)
        {
            delete[] mpCharArray;
        }
    }

    //! @brief Reallocate the buffer memmory (but only if new size is larger then before)
    //! @param[in] size The desired buffer size (the number of bytes to allocate)
    //! @returns true if reallocation was a success or if no reallocation was necessary, false if reallocation failed
    inline bool resize(size_t size)
    {
        if (size > mSize)
        {
            mpCharArray = static_cast<char*>(realloc(mpCharArray, size));
            if (mpCharArray)
            {
                mSize = size;
                return true;
            }
            else
            {
                mSize = 0;
                return false;
            }
        }
        // Lets keep the previously allocated memmory as buffer (to avoid time consuming realloc)
        return true;
    }

    //! @brief Returns the actual character buffer
    inline char* buff()
    {
        return mpCharArray;
    }

    //! @brief Returns the current buffer size
    inline size_t size()
    {
        return mSize;
    }

protected:
    char *mpCharArray;
    size_t mSize;
};

template <typename T>
bool IndexingCSVParser::getIndexedColumnAs(const size_t col, std::vector<T> &rData)
{
    return IndexingCSVParser::getIndexedColumnRowRangeAs<T>(col, 0, numRows(), rData);
}

template <typename T>
bool IndexingCSVParser::getIndexedColumnRowRangeAs(const size_t col, const size_t startRow, const size_t numRows, std::vector<T> &rData)
{
    // Assume all rows have same num cols
    if (col < numCols(startRow))
    {
        // Reserve data (will only increase reserved memmory if needed, not shrink)
        rData.reserve(numRows);

        // Temporary buffer object
        CharBuffer cb;

        // Loop through each row
        for (size_t r=startRow; r<startRow+numRows; ++r)
        {
            // Begin and end positions
            size_t b = mSeparatorPositions[r][col] + size_t(col > 0);
            size_t e = mSeparatorPositions[r][col+1];
            // Move file ptr
            std::fseek(mpFile, b, SEEK_SET);

            // Extract data
            cb.resize(e-b+1);
            char* rc = fgets(cb.buff(), e-b+1, mpFile);
            // Push back data
            if (rc)
            {
                bool parseOK;
                rData.push_back(converter<T>(cb.buff(), parseOK));
                if (!parseOK)
                {
                    return false;
                }
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

template <typename T>
bool IndexingCSVParser::getIndexedRowAs(const size_t row, std::vector<T> &rData)
{
    return IndexingCSVParser::getIndexedRowColumnRangeAs<T>(row,0,numCols(row),rData);
}

template <typename T>
bool IndexingCSVParser::getIndexedRowColumnRangeAs(const size_t row, const size_t startCol, const size_t numCols, std::vector<T> &rData)
{
    if (row < mSeparatorPositions.size())
    {
        // Reserve data (will only increase reserved memmory if needed, not shrink)
        rData.reserve(numCols);

        // Begin position
        size_t b = mSeparatorPositions[row][startCol] + 1*(startCol > 0);
        // Move file ptr
        fseek(mpFile, b, SEEK_SET);
        // Character buffer for extravtion and parsing
        CharBuffer cb;
        // Loop through each column on row
        for (size_t c=startCol+1; c<=startCol+numCols; ++c)
        {
            const size_t e = mSeparatorPositions[row][c];
            cb.resize(e-b+1);
            char* rc = fgets(cb.buff(), e-b+1, mpFile);
            if (rc)
            {
                bool parseOK;
                rData.push_back(converter<T>(cb.buff(), parseOK));
                if (!parseOK)
                {
                    return false;
                }
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


//! @brief Extract the data of a given indexed position row and column (as given template argument)
//! @tparam T The type do convert asci text to
//! @param[in] row The row index (0-based)
//! @param[in] col The column index (0-based)
//! @returns The value at the requested position as given templat type arguemnt default constructed value if position does not exist
template <typename T>
T IndexingCSVParser::getIndexedPosAs(const size_t row, const size_t col, bool &rParseOK)
{
    if (row < mSeparatorPositions.size())
    {
        if (col < mSeparatorPositions[row].size())
        {
            // Begin and end positions
            size_t b = mSeparatorPositions[row][col] + size_t(col > 0);
            size_t e = mSeparatorPositions[row][col+1];
            fseek(mpFile, b, SEEK_SET);

            char buff[e-b+1];
            char* rc = fgets(buff, e-b+1, mpFile);
            if (rc)
            {
                return converter<T>(buff, rParseOK);
            }
        }
    }
    rParseOK = false;
    return T();
}

template <typename T>
bool IndexingCSVParser::getRowAs(std::vector<T> &rData)
{
    bool isSuccess = true;
    size_t b = ftell(mpFile);
    while (true)
    {
        size_t e = ftell(mpFile);
        int c = fgetc(mpFile);

        if (c == mSeparatorChar || c == '\n' || c == '\r')
        {
            // Rewind file pointer to start of field
            fseek(mpFile, b, SEEK_SET);
            char buff[e-b+1];
            char* rc = fgets(buff, e-b+1, mpFile);
            if (rc)
            {
                bool parseOK;
                rData.push_back(converter<T>(buff, parseOK));
                // Indicate we failed to parse, but we still need to gobble the entire line incase we reach EOF
                if (!parseOK)
                {
                    isSuccess = false;
                }
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

            // Break loop when we have reachen EOL
            if (c == '\n')
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

    //! @todo try to index line first before extracting data, might be faster since we can reserve (maybe)
    return isSuccess;
}

}

#endif // INDEXINGCSVPARSERIMPL_HPP

