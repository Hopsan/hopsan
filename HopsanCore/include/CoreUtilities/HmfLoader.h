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
//! @file   HmfLoader.cc
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-03-20
//!
//! @brief Contains the HopsanCore HMF file load function
//!
//$Id$

#ifndef HMFLOADER_H_INCLUDED
#define HMFLOADER_H_INCLUDED


#include <vector>
#include <ctype.h>
#include "HopsanTypes.h"

namespace hopsan {

//Forward declaration
class ComponentSystem;
class HopsanEssentials;
class HopsanCoreMessageHandler;

inline int getGenerationVersion(const HString &version)
{
    HString tempStr;
    for(size_t i=0; i<version.size() && version.at(i) != '.'; ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    return tempStr.toLongInt(&dummy);
}

inline int getMajorVersion(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=0; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != '.'; ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    return tempStr.toLongInt(&dummy);
}

inline int getMinorVersion(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=0; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != 'x'; ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    if(tempStr == "")
        return -1;

    return tempStr.toLongInt(&dummy);
}


inline char getHotfixLetter(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=0; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != 'x'; ++i)
    {
        if(!isdigit(version.at(i)))
            tempStr.append(version.at(i));
    }

    if(tempStr.size() > 1)
        return ' ';

    return tempStr.at(0);
}


inline int getRevisionNumber(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=1; i<version.size() && version.at(i-1) != '_' && version.at(i) != 'r'; ++i) {}
    for(++i; i<version.size(); ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    if(tempStr == "")
        return -1;

    return tempStr.toLongInt(&dummy);
}


inline bool isVersionGreaterThan(HString version1, HString version2)
{
    int gen1 = getGenerationVersion(version1);
    int gen2 = getGenerationVersion(version2);
    int maj1 = getMajorVersion(version1);
    int maj2 = getMajorVersion(version2);
    int min1 = getMinorVersion(version1);
    int min2 = getMinorVersion(version2);
    char letter1 = getHotfixLetter(version1);
    char letter2 = getHotfixLetter(version2);
    int rev1 = getRevisionNumber(version1);
    int rev2 = getRevisionNumber(version2);

    if(gen1 > gen2)
        return true;
    if(gen1 < gen2)
        return false;

    if(maj1 > maj2)
        return true;
    if(maj1 < maj2)
        return false;

    if(min1 > -1 && min2 == -1)
        return false;               //Assume that revision build is higher generation than release builds
    if(min1 == -1 && min2 > -1)
        return true;

    if(min1 > min2)
        return true;
    if(min1 < min2)
        return false;

    if(letter1 > letter2)
        return true;
    if(letter1 < letter2)
        return false;

    if(rev1 > rev2)
        return true;
    if(rev1 < rev2)
        return false;

    return false;
}

ComponentSystem* loadHopsanModelFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModel(const std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModel(const char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModel(char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
void loadHopsanParameterFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem);

}

#endif

