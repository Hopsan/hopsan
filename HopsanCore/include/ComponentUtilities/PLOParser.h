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
//! @file   PLOParser.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-10-03
//!
//! @brief Contains the Core Utility PLOParser class
//!

#ifndef PLOPARSER_H
#define PLOPARSER_H

#include "win32dll.h"
#include <vector>
#include <iostream>
#include "HopsanTypes.h"

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class HOPSANCORE_DLLAPI PLOParser
{
public:
    PLOParser();

    void clearData();
    bool isEmpty() const;
    bool eof() const;

    bool readText(const HString &text);
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
    bool readFile(std::iostream& rFileStream);
    std::vector<double> mData;
    std::vector<HString> mPlotQuantitiesOrScales;
    std::vector<HString> mDataNames;
    size_t mNumDataRows, mNumDataCols, mPloVersion;
    HString mErrorString;
};

}

#endif // PLOPARSER_H
