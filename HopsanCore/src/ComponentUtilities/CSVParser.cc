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
//! @file   CSVParser.cc
//! @author Peter Nordin <peter.nordin@liu.se>
//!
//! @brief Contains the Core Utility CSVParser class
//!
//$Id$

#define INDCSVP_REPLACEDECIMALCOMMA
#include "IndexingCSVParser.h"

#include "ComponentUtilities/CSVParser.h"

using namespace hopsan;

CSVParserNG::CSVParserNG(const char separator_char)
{
    mpCsvParser = new indcsvp::IndexingCSVParser();
    mpCsvParser->setSeparatorChar(separator_char);
}

CSVParserNG::~CSVParserNG()
{
    mpCsvParser->closeFile();
    delete mpCsvParser;
}

bool CSVParserNG::openFile(const HString &rFilepath)
{
    return mpCsvParser->openFile(rFilepath.c_str());
}

void CSVParserNG::closeFile()
{
    mpCsvParser->closeFile();
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
