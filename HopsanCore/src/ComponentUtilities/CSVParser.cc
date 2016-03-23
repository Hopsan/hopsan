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

#include "csv_parser.hpp"
#define INDCSVP_REPLACEDECIMALCOMMA
#include "IndexingCSVParser.h"

#include "ComponentUtilities/CSVParser.h"
#include "ComponentUtilities/num2string.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace hopsan;

CSVParser::CSVParser(bool &rSuccess,
          const HString filename,
          const char line_terminator,
          const char enclosure_char)
{
    //Figure out field terminator and number of lines to skip
    char field_terminator;
    int lines_to_skip = 0;
    std::ifstream myfile(filename.c_str());
    if (myfile.is_open())
    {
        // Check that last char is a newline
        //! @todo it would be better if the parser lib could handle this
        char lastChar;
        myfile.seekg(-1, ios::end);
        myfile.get(lastChar);
        myfile.seekg(0, ios::beg);
        if (lastChar != '\n')
        {
            rSuccess = false;
            mErrorString = "No newline at end of file";
            return;
        }

        // Find first row with field separator
        std::string line = "";
        while( (line.find(',')==string::npos) && (line.find(';')==string::npos) )
        {
            getline(myfile,line);
            ++lines_to_skip;

            if (myfile.eof())
            {
                //We have reach the end lets give up
                rSuccess = false;
                mErrorString = "Could not find any separator signs";
                return;
            }
        }
        --lines_to_skip;
        myfile.close();

        // Select field separator ',' or ';'
        if( line.find(';')!=string::npos )
        {
            field_terminator = ';';     //Use semicolon
        }
        else
        {
            field_terminator = ',';     //Use comma
        }

        csv_parser file_parser;

        // Define how many records we're going skip. This could be used to skip the column definitions.
        file_parser.set_skip_lines(lines_to_skip);

        // Specify the file to parse
        rSuccess = file_parser.init(filename.c_str());
        if (rSuccess)
        {
            // Here we tell the parser how to parse the file
            file_parser.set_enclosed_char(enclosure_char, ENCLOSURE_OPTIONAL);
            file_parser.set_field_term_char(field_terminator);
            file_parser.set_line_term_char(line_terminator);

            mData.clear();
            mnDataRows=0;
            mnDataCols=0;
            size_t rowCtr=lines_to_skip;
            // Check to see if there are more records, then grab each row one at a time
            while( file_parser.has_more_rows() )
            {
                // Get the record
                csv_row row = file_parser.get_row();
                ++rowCtr;

                if(mData.empty())
                {
                    //Init data matrix, data will be stored column row wise (not row column as usual), this is for easier column access
                    mnDataCols = row.size();
                    mData.resize(mnDataCols);
                }
                else
                {
                    if (row.size() != mnDataCols)
                    {
                        rSuccess = false;
                        mErrorString = "Row: "+to_hstring(rowCtr)+" does not have the same number of columns as the previous rows";
                        break;
                    }
                }

                // Convert each column in the row
                for (size_t col=0; col<row.size(); ++col)
                {
                    // Extract a field string from row
                    string str = row[col];
                    // Replace decimal , with decimal .
                    replace(str.begin(), str.end(), ',', '.');
                    // Use a stream to stream value into double
                    double d;
                    std::istringstream is;
                    is.str(str);
                    is >> d;

                    // Append to each column
                    mData[col].push_back(d);
                }
            }

            if (!mData.empty())
            {
                mnDataRows = mData[0].size();
                setFirstLastValues();
                calcIncreasingOrDecreasing();
            }

            if (rSuccess && mData.empty())
            {
                rSuccess = false;
                mErrorString = "No data read from file";
            }
        }
        else
        {
            mErrorString = "csv_parser utility failed to initialize";
        }
    }
    else
    {
        rSuccess = false;
        mErrorString = "Could not open file";
    }
}

//! @brief Convenience function to set the first and last values in vector (to avoid lookup during simulation)
void CSVParser::setFirstLastValues()
{
    mFirstValues.resize(mnDataCols);
    mLastValues.resize(mnDataCols);
    for (size_t col=0; col<mnDataCols; ++col)
    {
        mFirstValues[col] = mData[col][0];
        mLastValues[col] = mData[col][mnDataRows-1];
    }
}

//! @brief Returns the last error message
HString CSVParser::getErrorString() const
{
    return mErrorString;
}

//! @brief Returns the number of loaded data rows
size_t CSVParser::getNumDataRows() const
{
    return mnDataRows;
}

//! @brief Returns the number of loaded data columns
size_t CSVParser::getNumDataCols() const
{
    return mnDataCols;
}

//! @brief Returns one full data column
//! @param [in] idx The index of the data column to retrieve
//! @returns Data column vector or empty vector if index out of range
const std::vector<double> CSVParser::getDataColumn(const size_t idx) const
{
    if (idx < mnDataCols)
    {
        return mData[idx];
    }
    return vector<double>();
}

//! @brief Returns if specified vector is strictly increasing or decreasing
//! @returns -1 on decreasing 1 on increasing 0 otherwise
int CSVParser::getIncreasingOrDecresing(const size_t idx) const
{
    if (idx < mnDataCols)
    {
        return mIncDec[idx];
    }
    return 0;
}

//! @brief Quicksort algorithm, based on WIKIPEDIA
size_t CSVParser::quickSortPartition(std::vector<double> &rIndexArray, const size_t left, const size_t right, const size_t pivotIndex)
{
    // left is the index of the leftmost element of the array
    // right is the index of the rightmost element of the array (inclusive)
    //   number of elements in subarray = right-left+1

    double pivotValue = rIndexArray[pivotIndex];
    swapRows(pivotIndex, right);  // Move pivot to end
    size_t storeIndex = left;
    for (size_t i=left; i<right; ++i)  // left ≤ i < right
    {
        if (rIndexArray[i] < pivotValue)
        {
            swapRows(i,storeIndex);
            storeIndex = storeIndex + 1;
        }
    }
    swapRows(storeIndex,right); // Move pivot to its final place
    return storeIndex;
}


//! @brief Quicksort algorithm, based on WIKIPEDIA
void CSVParser::quickSort(std::vector<double> &rIndexArray, const size_t left, const size_t right)
{
    // If the list has 2 or more items
    if (left < right)
    {
        // See "Choice of pivot" section below for possible choices
        //choose any 'pivotIndex' such that 'left' ≤ 'pivotIndex' ≤ 'right'
        size_t pivotIndex = left+(right-left)/2;

        // Get lists of bigger and smaller items and final position of pivot
        size_t pivotNewIndex = quickSortPartition(rIndexArray, left, right, pivotIndex);

        // Recursively sort elements smaller than the pivot
        // but not if it happened to be zero (would lead to underflow in size_t)
        if (pivotNewIndex>0)
        {
            quickSort(rIndexArray, left, pivotNewIndex-1);
        }

        // Recursively sort elements at least as big as the pivot
        quickSort(rIndexArray, pivotNewIndex+1, right);
    }
}

//! @brief Swaps the data on two rows
void CSVParser::swapRows(const size_t r1, const size_t r2)
{
    for (size_t col=0; col<mnDataCols; ++col)
    {
        double tmp = mData[col][r1];
        mData[col][r1] = mData[col][r2];
        mData[col][r2] = tmp;
    }
}

//! @brief Sorts the data so that the specified index vector is strictly increasing
void CSVParser::sortIncreasing(const size_t indexColumn)
{
    // If row is strictly decreasing the swap row order, else run quicksort and hope for the best
    if (mIncDec[indexColumn] == -1)
    {
        reverseRows();
        setFirstLastValues();
    }
    else if (mIncDec[indexColumn] == 0)
    {
        quickSort(mData[indexColumn], 0, mnDataRows-1);
        setFirstLastValues();
    }
    // Else we are already OK
}

//! @brief Check if data in columns are strictly increasing or decreasing
//! @todo maybe we should force the input data vector to be strictly increasing or decreasing
//! @todo maybe data should be automatically sorted when reading the file instead
void CSVParser::calcIncreasingOrDecreasing()
{
    if(!mData.empty())
    {
        mIncDec.resize(mnDataCols, 0);

        for(size_t col=0; col<mnDataCols; ++col)
        {
            bool increasing=true;
            bool decreasing=true;
            for(size_t row=1; row<mnDataRows; ++row)
            {
                if (mData[col][row] > mData[col][row-1])
                {
                    increasing = increasing && true;
                    decreasing = false;
                }

                if (mData[col][row] < mData[col][row-1])
                {
                    decreasing = decreasing && true;
                    increasing = false;
                }
            }

            if(increasing)
            {
                mIncDec[col] = 1;
            }

            if(decreasing)
            {
                mIncDec[col] = -1;
            }
        }
    }
}

//! @brief Reverses the row order
void CSVParser::reverseRows()
{
    vector< vector<double> > tempData;
    tempData.resize(mnDataCols, vector<double>(mnDataRows));

    for (size_t col=0; col<mnDataCols; ++col)
    {
        size_t rrow = mnDataRows-1;
        for (size_t row=0; rrow<mnDataRows; ++row)
        {
            tempData[col][rrow] = mData[col][row];
            --rrow;
        }
    }
    mData.swap(tempData);
}

//! @brief Recursively subdivide into halves to find start row for interpolation
//! @note This function assumes that the the data in column colIdx is strictly increasing
size_t CSVParser::intervalHalfSubDiv(const size_t colIdx, const double x, const size_t i1, const size_t iend) const
{
    if (iend-i1 <= 1)
    {
        // When the two indexes are next to each other lets return the smallest one as the start row for interpolation
        return i1;
    }
    else
    {
        //Calc split index
        size_t splitIdx = i1 + (iend - i1)/2; //Allow truncation

        if (x <= mData[colIdx][splitIdx])
        {
            // Use lower half
            return intervalHalfSubDiv(colIdx, x, i1, splitIdx);
        }
        else
        {
            // Use higher half
            return intervalHalfSubDiv(colIdx, x, splitIdx, iend);
        }
    }
}


//! @brief Check if inData column is strictly increasing or decreasing, otherwise interpolate will not work as expected
bool CSVParser::isInDataIncOrDec(const size_t inCol)
{
    if (inCol < mnDataCols)
    {
        if(mIncDec[inCol] != 0)
        {
            return true;
        }
        mErrorString = "Input data column is not strictly increasing or decreasing";
        return false;
    }
    mErrorString = "Input data column index out of range";
    return false;
}

//! @brief Interpolates, index vector lookup is using subdivision, assumes index column 0
//! @note Requires that the index vector is strictly increasing
//! @param [in] x The x value for interpolation
//! @param [in] outCol The column index for the output data
//! @returns The interpolated output value
double CSVParser::interpolate(const double x, const size_t outCol) const
{
    return interpolate(x, 0, outCol);
}

//! @brief Interpolates, index vector lookup is using subdivision
//! @note Requires that the index vector is strictly increasing
//! @param [in] x The x value for interpolation
//! @param [in] outCol The column index for the output data
//! @param [in] inCol The column index for the input (index) data
//! @returns The interpolated output value
double CSVParser::interpolate(const double x, const size_t inCol, const size_t outCol) const
{
    // Handle outside index range
    if( x<mFirstValues[inCol] )
    {
        return mFirstValues[outCol];
    }
    else if( x>=mLastValues[inCol] )
    {
        return mLastValues[outCol];
    }
    else
    {
        size_t row = intervalHalfSubDiv(inCol, x, 0, mnDataRows-1);
        if(mData[inCol][row+1] ==  mData[inCol][row])       //Check for division by zero (this means that if several X values have the same value, we will always pick the first one since we cannot interpolate between them)
        {
            return mData[outCol][row];
        }
        else
        {
            return mData[outCol][row] + (x - mData[inCol][row])*(mData[outCol][row+1] -  mData[outCol][row])/(mData[inCol][row+1] -  mData[inCol][row]);
        }

    }
    return x; //!< @todo  Don't know if this is correct, return x if we were unsuccessful
}


//! @brief Transposes the data matrix, useful if you know that data is stored line wise instead of column wise
void CSVParser::transpose()
{
    // Note! data is stored column wise in memory, so column / row is actually "transposed" compared to storage row / col

    //! @todo we really need to use smart vectors here
    vector< vector<double> > newData;
    newData.resize(mnDataCols);
    for (size_t i=0; i<mnDataCols; ++i)
    {
        newData[i].resize(mnDataRows);
    }

    for (size_t r=0; r<mnDataRows; ++r)
    {
        for (size_t c=0; c<mnDataCols; ++c)
        {
            newData[c][r] = mData[r][c];
        }
    }

    mData.swap(newData);

    size_t tmp = mnDataCols;
    mnDataCols = mnDataRows;
    mnDataRows = tmp;

    setFirstLastValues();
    calcIncreasingOrDecreasing();
}


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
    vector<double> wholeColRange;
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
