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
//! @date   2014-10-03
//!
//! @brief Contains the Core Utility PLOParser class
//!
//$Id$

#ifndef PLOPARSER_H
#define PLOPARSER_H

#include "win32dll.h"
#include <vector>
#include "HopsanTypes.h"

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT PLOParser
{
public:
    PLOParser();

    void clearData();
    bool isEmpty() const;
    bool eof() const;

    bool readFile(const HString &rFilepath);
    //! @todo maybe have a function that directly copies time and data

    HString getErrorString() const;
    size_t getNumDataRows() const;
    size_t getNumDataCols() const;
    size_t getPloFileVersion() const;

    std::vector<HString> getDataNames() const;
    int getColIdxForDataName(const HString &rName) const;

    bool copyRow(const size_t rowIdx, std::vector<double> &rRow);
    bool copyColumn(const size_t columnIdx, std::vector<double> &rColumn);
    bool copyColumn(const HString &rDataName, std::vector<double> &rColumn);

protected:
    std::vector<double> mData;
    std::vector<double> mPlotScales;
    std::vector<HString> mDataNames;
    size_t mNumDataRows, mNumDataCols, mPloVersion;
    HString mErrorString;
};

}

#endif // PLOPARSER_H
