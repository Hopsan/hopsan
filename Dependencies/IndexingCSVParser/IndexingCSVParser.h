//!
//! @file   IndexingCSVParser.h
//! @author Peter Nordin
//! @date   2015-02-03
//!
//! @brief Contains the definition of the IndexingCSVParser
//!

#ifndef INDEXINGCSVPARSER_H
#define INDEXINGCSVPARSER_H

#include <string>
#include <vector>

//! @brief The Indexing CSV Parser namespace
namespace indcsvp
{

//! @brief The Indexing CSV Parser Class
class IndexingCSVParser
{
public:
    IndexingCSVParser();

    // ----- Configuration methods -----
    void setSeparatorChar(char sep);
    void setCommentChar(char com);
    void setNumLinesToSkip(size_t num);
    char autoSetSeparatorChar(const std::vector<char> &rAlternatives);
    char getSeparatorChar() const;
    char getCommentChar() const;
    size_t getNumLinesToSkip() const;

    // ----- File methods -----
    bool openFile(const char* filePath);
    void closeFile();
    void rewindFile();
    void readUntilData();

    // ----- Indexing access methods -----
    void indexFile();

    size_t numRows() const;
    size_t numCols(size_t row=0) const;
    bool allRowsHaveSameNumCols() const;
    void minMaxNumCols(size_t &rMin, size_t &rMax);

    bool getIndexedColumn(const size_t col, std::vector<std::string> &rData);
    bool getIndexedRow(const size_t row, std::vector<std::string> &rData);
    std::string getIndexedPos(const size_t row, const size_t col, bool &rParseOK);

    template <typename T> bool getIndexedColumnAs(const size_t col, std::vector<T> &rData);
    template <typename T> bool getIndexedColumnRowRangeAs(const size_t col, const size_t startRow, const size_t numRows, std::vector<T> &rData);
    template <typename T> bool getIndexedRowAs(const size_t row, std::vector<T> &rData);
    template <typename T> bool getIndexedRowColumnRangeAs(const size_t row, const size_t startCol, const size_t numCols, std::vector<T> &rData);
    template <typename T> T getIndexedPosAs(const size_t row, const size_t col, bool &rParseOK);

    // ----- Non-indexing access methods -----
    bool getRow(std::vector<std::string> &rData);
    template <typename T> bool getRowAs(std::vector<T> &rData);
    bool hasMoreDataRows();

protected:
    FILE *mpFile;           //!< @brief The internal file pointer
    char mSeparatorChar;    //!< @brief The chosen separator character
    char mCommentChar;      //!< @brief The chosen comment character
    size_t mNumSkipLines;   //!< @brief The initial lines to skip
    std::vector< std::vector<size_t> > mSeparatorPositions; //!< @brief The index of separators
};

}

// Include the header-only implementation (template functions and methods)
#include "IndexingCSVParserImpl.hpp"

#endif // INDEXINGCSVPARSER_H
