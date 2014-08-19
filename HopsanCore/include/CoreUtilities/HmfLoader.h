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
ComponentSystem* loadHopsanModelFile(const std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModelFile(const char* xmlStr, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModelFile(char* xmlStr, HopsanEssentials* pHopsanEssentials);
void loadHopsanParameterFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem);

}

#endif

