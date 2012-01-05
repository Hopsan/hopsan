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
#include "Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp"
#include <algorithm>
#include <sstream>

using namespace hopsan;

CSVParser::CSVParser(bool &success,
          const std::string filename,
          const char field_terminator,
          const char line_terminator,
          const char enclosure_char,
          const size_t linesToSkip)
{

    csv_parser file_parser;

    // Define how many records we're gonna skip. This could be used to skip the column definitions.
    file_parser.set_skip_lines(linesToSkip);

    // Specify the file to parse
    success = file_parser.init(filename.c_str());

    // Here we tell the parser how to parse the file
    file_parser.set_enclosed_char(enclosure_char, ENCLOSURE_OPTIONAL);

    file_parser.set_field_term_char(field_terminator);

    file_parser.set_line_term_char(line_terminator);

    unsigned int row_count = 0;

    bool mDataIsInited = false;

    // Check to see if there are more records, then grab each row one at a time
    while(file_parser.has_more_rows())
    {
            unsigned int i = 0;

            // Get the record
            csv_row row = file_parser.get_row();

            if(!mDataIsInited)
            {
                mData.resize(row.size());
                mDataIsInited = true;
            }

            // Print out each column in the row
            for (i = 0; i < row.size(); i++)
            {
                std::istringstream is;
                string s = row[i];
                replace(s.begin(), s.end(), ',', '.');
                is.str(s);
                double d;
                is >> d;
                mData[i].push_back(d);
            }

            row_count++;
    }

}

double CSVParser::interpolate(const double x, const size_t outIndex, const size_t inIndex)
{
    size_t i;
    //! @todo remove this stupid loop and use direct indexing instead
    for (i=0; i < mData[inIndex].size()-1; i++)
    {
        if(((x <= mData[inIndex][i]) && (x > mData[inIndex][i+1])) ||
           ((x >= mData[inIndex][i]) && (x < mData[inIndex][i+1])))
        {
            //Value is between i and i+1
            //return  mData[outIndex][i];
            return mData[outIndex][i] + (x - mData[inIndex][i])*(mData[outIndex][i+1] -  mData[outIndex][i])/(mData[inIndex][i+1] -  mData[inIndex][i]);
            break;
        }
    }
    return x; //!< @todo  Dont know if this is correct, return x if we vere unsucessfull
}
