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

int getEpochVersion(const HString& version);
int getMajorVersion(const HString& version);
int getMinorVersion(const HString& version);
bool isVersionGreaterThan(const HString& version1, const HString& version2);
int compareHopsanVersions(const HString& version1, const HString& version2);

ComponentSystem* loadHopsanModelFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModel(const std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModel(const char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModel(char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
void loadHopsanParameterFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem);

}

#endif

