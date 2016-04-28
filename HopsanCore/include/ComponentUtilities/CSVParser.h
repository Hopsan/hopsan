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
//! @author Peter Nordin <peter.nordin@liu.se>
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
namespace indcsvp
{
class IndexingCSVParser;
}

namespace hopsan {

//! @ingroup ComponentUtilityClasses
//! @brief The CSV file parser utility
class DLLIMPORTEXPORT CSVParserNG
{
public:
    CSVParserNG(const char separator_char = ',');
    ~CSVParserNG();

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
