/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   CSVParser.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-12
//!
//! @brief Contains the Core Utility CSVParser class
//!
//$Id$

#ifndef CVSPARSER_H_INCLUDED
#define CVSPARSER_H_INCLUDED

#include "win32dll.h"
#include <vector>
#include "HopsanTypes.h"

// Forward declaration
class csv_parser;
namespace indcsvp
{
class IndexingCSVParser;
}

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT CSVParser
{
public:
    CSVParser(bool &rSuccess,
              const HString filename,
              const char line_terminator = '\n',
              const char enclosure_char = '"');

    HString getErrorString() const;
    size_t getNumDataRows() const;
    size_t getNumDataCols() const;
    const std::vector<double> getDataColumn(const size_t idx) const;

    int getIncreasingOrDecresing(const size_t idx) const;
    void calcIncreasingOrDecreasing();
    void sortIncreasing(const size_t indexColumn);

    bool isInDataIncOrDec(const size_t inCol);
    double interpolate(const double x, const size_t outCol) const;
    double interpolate(const double x, const size_t inCol, const size_t outCol) const;

    void transpose();

protected:
    size_t intervalHalfSubDiv(const size_t colIdx, const double x, const size_t i1, const size_t iend) const;
    size_t quickSortPartition(std::vector<double> &rIndexArray, const size_t left, const size_t right, const size_t pivotIndex);
    void quickSort(std::vector<double> &rIndexArray, const size_t left, const size_t right);
    void swapRows(const size_t r1, const size_t r2);
    void reverseRows();
    void setFirstLastValues();

    std::vector< std::vector<double> > mData;
    std::vector<double> mFirstValues, mLastValues;
    size_t mnDataRows, mnDataCols;
    std::vector< int > mIncDec;
    HString mErrorString;
};

class DLLIMPORTEXPORT CSVParserNG
{
public:
    CSVParserNG(const char line_terminator = '\n', const char enclosure_char = '"');
    ~CSVParserNG();

    void clearData();
    bool isEmpty() const;

    void setLineTerminator(const char lt);
    void setFieldEnclosureChar(const char fec);

    bool setFile(const HString &rFilepath);

    bool parseEntireFile();

    bool eof() const;

    HString getErrorString() const;
    size_t getNumDataRows() const;
    size_t getNumDataCols() const;

    bool copyRow(const size_t rowIdx, std::vector<double> &rRow);
    bool copyRow(const size_t rowIdx, std::vector<long int> &rRow);
    bool copyColumn(const size_t columnIdx, std::vector<double> &rColumn);
    bool copyRangeFromColumn(const size_t columnIdx, const size_t startRow, const size_t numRows, std::vector<double> &rColumn);
    bool copyEveryNthFromColumn(const size_t columnIdx, const size_t stepSize, std::vector<double> &rColumn);
    bool copyEveryNthFromColumnRange(const size_t columnIdx, const size_t startRow, const size_t numRows, const size_t stepSize, std::vector<double> &rColumn);

protected:
    std::vector<HString> mData;
    size_t mNumDataRows, mNumDataCols;

    char mLineTerminator, mFieldEnclosureChar, mFieldSeparator;
    size_t mNumLinesToSkip;
    csv_parser *mpCsvParser;

    HString mErrorString;
    bool mConvertDecimalSeparator;
};



class DLLIMPORTEXPORT CSVParserNNG
{
public:
    CSVParserNNG(const char separator_char = ',');
    ~CSVParserNNG();

    bool openFile(const HString &rFilepath);
    void closeFile();

    void setFieldSeparator(const char sep);
    char autoSetFieldSeparator(std::vector<char> &rAlternatives);

    void indexFile();
    size_t getNumDataRows() const;
    size_t getNumDataCols(const size_t row=0) const;
    void getMinMaxNumCols(size_t &rMin, size_t &rMax) const;
    bool allRowsHaveSameNumCols() const;

    HString getErrorString() const;

    bool copyRow(const size_t rowIdx, std::vector<double> &rRow);
    bool copyRow(const size_t rowIdx, std::vector<long int> &rRow);
    bool copyColumn(const size_t columnIdx, std::vector<double> &rColumn);
    bool copyRangeFromColumn(const size_t columnIdx, const size_t startRow, const size_t numRows, std::vector<double> &rColumn);
    bool copyEveryNthFromColumn(const size_t columnIdx, const size_t stepSize, std::vector<double> &rColumn);
    bool copyEveryNthFromColumnRange(const size_t columnIdx, const size_t startRow, const size_t numRows, const size_t stepSize, std::vector<double> &rColumn);

protected:
    indcsvp::IndexingCSVParser *mpCsvParser;
    HString mErrorString;
    bool mConvertDecimalSeparator;
};

}

#endif // CVSPARSER_H_INCLUDED
