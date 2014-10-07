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
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-10-03
//!
//! @brief Contains the Core Utility PLOParser class
//!
//$Id: CSVParser.h 7179 2014-06-23 15:01:03Z petno25 $

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
    //! @todo maybe have a function that direcly copies time and data

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
