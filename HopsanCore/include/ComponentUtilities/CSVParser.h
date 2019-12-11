/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include <cstdio>
#include "HopsanTypes.h"

// Forward declaration
namespace indcsvp
{
class IndexingCSVParser;
}

namespace hopsan {

//! @ingroup ComponentUtilityClasses
//! @brief The CSV file parser utility
class HOPSANCORE_DLLAPI CSVParserNG
{
public:
    CSVParserNG(const char separator_char = ',', size_t linesToSkip=0);
    ~CSVParserNG();

    bool openText(HString text);
    bool openFile(const HString &rFilepath);
    bool takeOwnershipOfFile(FILE* pFile);
    void closeFile();

    void setCommentChar(char commentChar);
    void setLinesToSkip(size_t linesToSkip);
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
