/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   CSVParser.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-11-01
//!
//! @brief Contains the Core Utility CSVParser class
//!
//$Id$

#include "ComponentUtilities/CSVParser.h"
#include "csv_parser.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace hopsan;

CSVParser::CSVParser(bool &rSuccess,
          const std::string filename,
          const char line_terminator,
          const char enclosure_char)
{
    //Figure out field terminator and number of lines to skip
    char field_terminator;
    int lines_to_skip = 0;
    std::string line = "";
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
        while( line.find(',')==string::npos && line.find(';')==string::npos )
        {
            getline(myfile,line);
            ++lines_to_skip;
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

        // Define how many records we're gonna skip. This could be used to skip the column definitions.
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
                        stringstream ss;
                        ss << "Row: " << rowCtr << " does not have the same number of columns as the previous rows";
                        mErrorString = ss.str();
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

            if (mData.size() > 0)
            {
                mnDataRows = mData[0].size();
                mFirstValues.resize(mnDataCols);
                mLastValues.resize(mnDataCols);
                for (size_t col=0; col<mnDataCols; ++col)
                {
                    mFirstValues[col] = mData[col][0];
                    mLastValues[col] = mData[col][mnDataRows-1];
                }
            }

            calcIncreasingOrDecreasing();
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

//! @brief Returns tha last error message
std::string CSVParser::getErrorString() const
{
    return mErrorString;
}

size_t CSVParser::getNumDataRows() const
{
    return mnDataRows;
}

size_t CSVParser::getNumDataCols() const
{
    return mnDataCols;
}

const std::vector<double> CSVParser::getDataColumn(const size_t idx) const
{
    if (idx < mnDataCols)
    {
        return mData[idx];
    }
    return vector<double>();
}

int CSVParser::getIncreasingOrDecresing(const size_t idx) const
{
    if (idx < mnDataCols)
    {
        return mIncDec[idx];
    }
    return 0;
}

//! @brief Check if data in columns are strictly increasing or decresing
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

//! @brief Check if inData column is strictly increasing or decreasing, otherwise interpolate will not work as expected
bool CSVParser::isInDataOk(const size_t inCol)
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

double CSVParser::interpolate(const double x, const size_t outCol, const size_t inCol) const
{
    // Handle outside index range
    if( ((x<mFirstValues[inCol]) && (mIncDec[inCol]==1)) || ((x>mFirstValues[inCol]) && (mIncDec[inCol]==-1)) )
    {
        return mFirstValues[outCol];
    }
    else if( ((x>=mLastValues[inCol]) && (mIncDec[inCol]==1)) || ((x<=mLastValues[inCol]) && (mIncDec[inCol]==-1)) )
    {
        return mLastValues[outCol];
    }
    else
    {
        //! @todo remove this stupid loop and use direct indexing instead
        for (size_t row=0; row<mnDataRows-1; row++)
        {
            if( ((x <= mData[inCol][row]) && (x > mData[inCol][row+1])) || ((x >= mData[inCol][row]) && (x < mData[inCol][row+1])) )
            {
                //Value is between i and i+1
                return mData[outCol][row] + (x - mData[inCol][row])*(mData[outCol][row+1] -  mData[outCol][row])/(mData[inCol][row+1] -  mData[inCol][row]);
            }
        }
    }
    return x; //!< @todo  Dont know if this is correct, return x if we vere unsucessfull
}

double CSVParser::interpolateInc(const double x, const size_t outCol, const size_t inCol) const
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
        //! @todo remove this stupid loop and use direct indexing instead
        for (size_t row=0; row<mnDataRows-1; row++)
        {
            // Ceeck if value is between i and i+1
            if( (x >= mData[inCol][row]) && (x < mData[inCol][row+1]) )
            {
                return mData[outCol][row] + (x - mData[inCol][row])*(mData[outCol][row+1] -  mData[outCol][row])/(mData[inCol][row+1] -  mData[inCol][row]);
            }
        }
    }
    return x; //!< @todo  Dont know if this is correct, return x if we vere unsucessfull
}
