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
            bool dataIsInited = false;
            size_t nDataCols=0;
            size_t rowCtr=lines_to_skip;
            vector<double> dbl_row;
            // Check to see if there are more records, then grab each row one at a time
            while( file_parser.has_more_rows() )
            {
                // Get the record
                csv_row row = file_parser.get_row();
                ++rowCtr;

                if(dataIsInited)
                {
                    if (row.size() != nDataCols)
                    {
                        rSuccess = false;
                        stringstream ss;
                        ss << "Row: " << rowCtr << " does not have the same length as the previous rows";
                        mErrorString = ss.str();
                        break;
                    }
                }
                else
                {
                    nDataCols = row.size();
                    dbl_row.resize(nDataCols,0);
                    dataIsInited = true;
                }

                // Convert each column in the row
                for (size_t i=0; i<row.size(); ++i)
                {
                    // Extract a field string from row
                    string str = row[i];
                    // Replace decimal , with decimal .
                    replace(str.begin(), str.end(), ',', '.');
                    // Use a stream to stream value into double
                    std::istringstream is;
                    is.str(str);
                    is >> dbl_row[i];
                }

                mData.push_back(dbl_row);
            }

            if (mData.size() > 0)
            {
                mFirstValues.resize(nDataCols);
                mLastValues.resize(nDataCols);
                for (size_t col=0; col<nDataCols; ++col)
                {
                    mFirstValues[col] = mData[0][col];
                    mLastValues[col] = mData[mData.size()-1][col];
                }
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

//! @brief Returns tha last error message
std::string CSVParser::getErrorString() const
{
    return mErrorString;
}

//! @todo what is this function suposed to do, clean it up also
bool CSVParser::checkData()
{
    bool isOk = false;
    if(!mData.empty())
    {
        const size_t nRows = mData.size();
        const size_t nCols = mData[0].size();
        mIncreasing.resize(nCols, 0);

        for(size_t col=0; col<nCols; ++col)
        {
            bool increasing=true;
            bool decreasing=true;
            for(size_t row=1; row<nRows; ++row)
            {
                if (mData[row][col] > mData[row-1][col])
                {
                    increasing = increasing && true;
                    decreasing = false;
                }

                if (mData[row][col] < mData[row-1][col])
                {
                    decreasing = decreasing && true;
                    increasing = false;
                }
            }

            if(increasing)
            {
                mIncreasing[col] = 1;
            }

            if(decreasing)
            {
                mIncreasing[col] = -1;
            }
        }
        //! @todo maybe we should force the input data vector to be strictly increasing or incresing
        isOk = true;
    }
    return isOk;
}


double CSVParser::interpolate(bool &okInIndex, const double x, const size_t outIndex, const size_t inIndex)
{
    //okInIndex =
    if(mIncreasing[inIndex] != 0)
    {
        okInIndex = true;
    }
    else
    {
        okInIndex = false;
    }
    if(okInIndex)
    {
        size_t i;
        //out of index
        if( ((x<mFirstValues[inIndex]) && (mIncreasing[inIndex]==1)) || ((x>mFirstValues[inIndex]) && (mIncreasing[inIndex]==-1)) )
        {
            return mFirstValues[outIndex];
        }
        else if( ((x>=mLastValues[inIndex]) && (mIncreasing[inIndex]==1)) || ((x<=mLastValues[inIndex]) && (mIncreasing[inIndex]==-1)) )
        {
            return mLastValues[outIndex];
        }
        else
        {
            //! @todo remove this stupid loop and use direct indexing instead
            for (i=0; i < mData.size()-1; i++)
            {
                if( ((x <= mData[i][inIndex]) && (x > mData[i+1][inIndex])) || ((x >= mData[i][inIndex]) && (x < mData[i+1][inIndex])) )
                {
                    //Value is between i and i+1
                    //return  mData[outIndex][i];
                    return mData[i][outIndex] + (x - mData[i][inIndex])*(mData[i+1][outIndex] -  mData[i][outIndex])/(mData[i+1][inIndex] -  mData[i][inIndex]);
                    break;
                }
            }
        }
    }
    return x; //!< @todo  Dont know if this is correct, return x if we vere unsucessfull
}
