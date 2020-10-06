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
//! @file   PLOParser.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-10-03
//!
//! @brief Contains the Core Utility PLOParser class
//!

#include "ComponentUtilities/PLOParser.h"
#include "ComponentUtilities/num2string.hpp"

#include <fstream>
#include <cstring>

using namespace hopsan;

PLOParser::PLOParser()
{
    clearData();
}

void PLOParser::clearData()
{
    mData.clear();
    mDataNames.clear();
    mErrorString.clear();
    mPlotQuantitiesOrScales.clear();
    mPloVersion = 0;
    mNumDataCols = 0;
    mNumDataRows = 0;

}

bool PLOParser::isEmpty() const
{
    return mData.empty();
}

bool PLOParser::readText(const HString &text)
{
   std::stringbuf string_buffer;
   std::iostream ios(&string_buffer);
   ios << text.c_str();
   ios.seekg(0);
   return readFile(ios);
}

bool PLOParser::readFile(const HString &rFilepath)
{
    // Attempt to open file
    std::fstream myfile(rFilepath.c_str());
    // Attempt to open file
    if (myfile.is_open()) {
        bool readOK = readFile(myfile);
        myfile.close();
        return readOK;
    }
    else {
        mErrorString = "Could not open file: "+rFilepath;
        myfile.close();
        return false;
    }
}

bool PLOParser::readFile(std::iostream &rFileStream)
{
    // Clear old data
    clearData();

    std::string tmp;

    // Read version data and check if this seems to be a plo file
    rFileStream >> tmp;
    if ( tmp != "'VERSION'" )
    {
        mErrorString = "Plo file does not begin with 'VERSION', or not a valid plo format, Aborting import!";
        return false;
    }

    // Read version number
    rFileStream >> mPloVersion;
    if (mPloVersion < 1 || mPloVersion > 2)
    {
        mErrorString = "Incorrect PLO version: " + to_hstring(mPloVersion);
        return false;
    }


    // Skip name line
    getline(rFileStream, tmp); // First remove previous newline
    getline(rFileStream, tmp);
    //! @todo maybe parse this for model info for plo v2 format

    // Read num data
    rFileStream >> mNumDataCols;
    rFileStream >> mNumDataRows;

    if ( mNumDataCols < 1 || mNumDataRows < 1 )
    {
        mErrorString = "Number of data rows or columns is less than one";
        return false;
    }

    // Read column names
    const bool isPloV1orV2 = (mPloVersion == 1) || (mPloVersion == 2);

    mDataNames.reserve(mNumDataCols+1); // Reserve one extra in case plo v1 or v2 and Time in first column
    for (size_t i=0; i<mNumDataCols; ++i)
    {
        rFileStream >> tmp;
        HString name(tmp.c_str());

        // PLO v1 and v2 does not count the "Time" or "Frequency" vector if it is in column 1, so add one to the number of columns in that case
        if ( (i==0) && isPloV1orV2 && ((name=="'Time',") || (name=="'Frequency',")) ) {
            ++mNumDataCols;
        }
        //! @todo Use proper splitting on , and then trimm
        // Remove ', and ' from names (including , separator)
        name.replace("',", "");
        name.replace("'", "");

        // Remember name
        mDataNames.push_back(name);
    }

    // Read plotscales
    mPlotQuantitiesOrScales.reserve(mNumDataCols);
    for (size_t i=0; i<mNumDataCols; ++i)
    {
        rFileStream >> tmp;
        mPlotQuantitiesOrScales.push_back(HString(tmp.c_str()));
    }

    // Read data
    mData.resize(mNumDataCols*mNumDataRows);
    for (size_t i=0; i<mNumDataCols*mNumDataRows; ++i)
    {
        double val;
        rFileStream >> val;
        mData[i] = val;
    }

    if (mPloVersion == 1 || mPloVersion == 2 )
    {
        // Read DAT line (not used, ignored in HopsanNG)
        rFileStream >> tmp;

        // Read modelname
        rFileStream >> tmp;
        //! @todo should we save this?
    }

    // All good
    return true;
}

HString PLOParser::getErrorString() const
{
    return mErrorString;
}

size_t PLOParser::getNumDataRows() const
{
    return mNumDataRows;
}

size_t PLOParser::getNumDataCols() const
{
    return mNumDataCols;
}

size_t PLOParser::getPloFileVersion() const
{
    return mPloVersion;
}

std::vector<HString> PLOParser::getDataNames() const
{
    return mDataNames;
}

int PLOParser::getColIdxForDataName(const HString &rName) const
{
    for (size_t i=0; i<mDataNames.size(); ++i)
    {
        if (rName == mDataNames[i])
        {
            return int(i);
        }
    }
    return -1;
}

bool PLOParser::copyRow(const size_t rowIdx, std::vector<double> &rRow)
{
    if (rowIdx < mNumDataRows)
    {
        rRow.resize(mNumDataCols);
        for (size_t c=0; c<mNumDataCols; ++c)
        {
            rRow[c] = mData[rowIdx*mNumDataCols+c];
        }
        return true;
    }
    return false;
}

bool PLOParser::copyColumn(const size_t columnIdx, std::vector<double> &rColumn)
{
    if (columnIdx < mNumDataCols)
    {
        rColumn.resize(mNumDataRows);
        for (size_t r=0; r<mNumDataRows; ++r)
        {
            rColumn[r] = mData[r*mNumDataCols+columnIdx];
        }
        return true;
    }
    return false;
}

bool PLOParser::copyColumn(const HString &rDataName, std::vector<double> &rColumn)
{
    int cid = getColIdxForDataName(rDataName);
    if (cid >= 0)
    {
        return copyColumn(size_t(cid), rColumn);
    }
    return false;
}
