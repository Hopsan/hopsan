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

#include "win32dll.h"
#include <string>
#include <vector>

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT CSVParser
{
public:
    CSVParser(bool &rSuccess,
              const std::string filename,
              const char line_terminator = '\n',
              const char enclosure_char = '"');

    const std::vector<double> getDataColumn(const size_t idx) const;
    int getIncreasingOrDecresing(const size_t idx) const;

    bool isInDataOk(const size_t inCol);
    double interpolate_old(const double x, const size_t outCol, const size_t inCol=0) const;
    double interpolate(const double x, const size_t outCol, const size_t inCol=0) const;
    double interpolateInc(const double x, const size_t outCol, const size_t inCol=0) const;
    double interpolateIncSubDiv(const double x, const size_t outCol, const size_t inCol=0) const;

    std::string getErrorString() const;
    size_t getNumDataRows() const;
    size_t getNumDataCols() const;

protected:
    size_t intervalHalfSubDiv(const size_t colIdx, const double x, const size_t i1, const size_t iend) const;
    size_t intervalQuadSubDiv(const size_t colIdx, const double x, const size_t i1, const size_t iend) const;
    void calcIncreasingOrDecreasing();

    std::vector< std::vector<double> > mData;
    std::vector<double> mFirstValues, mLastValues;
    size_t mnDataRows, mnDataCols;
    std::vector< int > mIncDec;
    std::string mErrorString;
};

}

#endif // CVSPARSER_H_INCLUDED
