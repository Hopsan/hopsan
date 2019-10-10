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
//! @file   CSVParser.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//!
//! @brief Contains the Core Utility CSVParser class
//!
//$Id$

#define INDCSVP_REPLACEDECIMALCOMMA
#include "indexingcsvparser/indexingcsvparser.h"

#include "ComponentUtilities/CSVParser.h"
#include <fstream>

#ifdef _WIN32
#include "windows.h"
#endif


using namespace hopsan;

CSVParserNG::CSVParserNG(const char separator_char, size_t linesToSkip)
{
    mpCsvParser = new indcsvp::IndexingCSVParser();
    mpCsvParser->setSeparatorChar(separator_char);
    mpCsvParser->setNumLinesToSkip(linesToSkip);
}

CSVParserNG::~CSVParserNG()
{
    mpCsvParser->closeFile();
    delete mpCsvParser;
}

bool CSVParserNG::openText(HString text)
{
    char tempFileBuffer[L_tmpnam ];
#ifdef _WIN32
    char tempPathBuffer[MAX_PATH];
    tmpnam(tempFileBuffer);
    DWORD len = GetTempPathA(MAX_PATH, tempPathBuffer);
    mTmpFileName.setString(tempPathBuffer, len);
    mTmpFileName.append(tempFileBuffer);
#else
    tmpnam(tempFileBuffer);
    mTmpFileName.setString(tempFileBuffer);
#endif

    try {
        std::ofstream tmpFile;
        tmpFile.open(mTmpFileName.c_str(), std::ofstream::out | std::ofstream::app);
        tmpFile << text.c_str();
        tmpFile.close();
    } catch (std::exception& e) {
        mErrorString = e.what();
        return false;
    }

    return openFile(mTmpFileName.c_str());
}

bool CSVParserNG::openFile(const HString &rFilepath)
{
    return mpCsvParser->openFile(rFilepath.c_str());
}

void CSVParserNG::closeFile()
{
    mpCsvParser->closeFile();
    if (!mTmpFileName.empty()) {
        remove(mTmpFileName.c_str());
        mTmpFileName.clear();
    }
}

void CSVParserNG::setFieldSeparator(const char sep)
{
    mpCsvParser->setSeparatorChar(sep);
}

char CSVParserNG::autoSetFieldSeparator(std::vector<char> &rAlternatives)
{
    return mpCsvParser->autoSetSeparatorChar(rAlternatives);
}

void CSVParserNG::indexFile()
{
    mpCsvParser->indexFile();
}

size_t CSVParserNG::getNumDataRows() const
{
    return mpCsvParser->numRows();
}

size_t CSVParserNG::getNumDataCols(const size_t row) const
{
    return mpCsvParser->numCols(row);
}

bool CSVParserNG::allRowsHaveSameNumCols() const
{
    return mpCsvParser->allRowsHaveSameNumCols();
}

void CSVParserNG::getMinMaxNumCols(size_t &rMin, size_t &rMax) const
{
    return mpCsvParser->minMaxNumCols(rMin, rMax);
}

HString CSVParserNG::getErrorString() const
{
    return mErrorString;
}

bool CSVParserNG::copyRow(const size_t rowIdx, std::vector<double> &rRow)
{
    if (rowIdx < mpCsvParser->numRows())
    {
        return mpCsvParser->getIndexedRowAs<double>(rowIdx, rRow);
        //! @todo convert decimal separator
    }
    else
    {
        mErrorString = "rowIdx out of range";
        return false;
    }
}

bool CSVParserNG::copyRow(const size_t rowIdx, std::vector<long int> &rRow)
{
    if (rowIdx < mpCsvParser->numRows())
    {
        return mpCsvParser->getIndexedRowAs<long int>(rowIdx, rRow);
    }
    else
    {
        mErrorString = "rowIdx out of range";
        return false;
    }
}

bool CSVParserNG::copyColumn(const size_t columnIdx, std::vector<double> &rColumn)
{
    if (mpCsvParser->numRows() > 0)
    {
        return copyRangeFromColumn(columnIdx, 0, mpCsvParser->numRows(), rColumn);
    }
    else
    {
        mErrorString = "To few rows < 1";
        return false;
    }
}

bool CSVParserNG::copyRangeFromColumn(const size_t columnIdx, const size_t startRow, const size_t numRows, std::vector<double> &rColumn)
{
    rColumn.clear();

    //! @todo assumes that all rows have same num cols
    if (columnIdx < mpCsvParser->numCols(startRow))
    {
        return mpCsvParser->getIndexedColumnRowRangeAs<double>(columnIdx, startRow, numRows, rColumn);
    }
    else
    {
        mErrorString = "columnIdx out of range";
        return false;
    }
}

bool CSVParserNG::copyEveryNthFromColumn(const size_t columnIdx, const size_t stepSize, std::vector<double> &rColumn)
{
    return copyEveryNthFromColumnRange(columnIdx, 0, mpCsvParser->numRows(), stepSize, rColumn);
}

bool CSVParserNG::copyEveryNthFromColumnRange(const size_t columnIdx, const size_t startRow, const size_t numRows, const size_t stepSize, std::vector<double> &rColumn)
{
    rColumn.clear();
    std::vector<double> wholeColRange;
    bool rc = mpCsvParser->getIndexedColumnRowRangeAs<double>(columnIdx, startRow, numRows, wholeColRange);
    if (rc)
    {
        rColumn.reserve(numRows/stepSize);
        for (size_t r=0; r<wholeColRange.size(); r+=stepSize)
        {
            rColumn.push_back(wholeColRange[r]);
        }
        return true;
    }
    else
    {
        mErrorString = "Failed to get data";
        return false;
    }
}
