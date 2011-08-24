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
//! @file   CSVParser.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-12
//!
//! @brief Contains the Core Utility CSVParser class
//!
//$Id$

#ifndef CVSPARSER_H_INCLUDED
#define CVSPARSER_H_INCLUDED

#include "../win32dll.h"
#include "assert.h"
#include "math.h"
#include <vector>
#include <string>
#include "../../ExternalDependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace hopsan {

    class DLLIMPORTEXPORT CSVParser
    {
    public:
        CSVParser(bool &success,
                  std::string filename = "Book1.csv",
                  const char field_terminator = ';',
                  const char line_terminator = '\n',
                  const char enclosure_char = '"',
                  size_t linesToSkip = 0)
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

        double interpolate(double x, size_t outIndex = 1, size_t inIndex = 0)
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

        }


    public: //Should be protected:
        std::vector< std::vector<double> > mData;
    };


}

#endif // CVSPARSER_H_INCLUDED
