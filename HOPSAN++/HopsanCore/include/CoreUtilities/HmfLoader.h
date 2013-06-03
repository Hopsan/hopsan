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
#include "HopsanTypes.h"

namespace hopsan {

//Forward declaration
class ComponentSystem;
class HopsanEssentials;
class HopsanCoreMessageHandler;


ComponentSystem* loadHopsanModelFile(const HString filePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime);
ComponentSystem* loadHopsanModelFile(const std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModelFile(const char* xmlStr, HopsanEssentials* pHopsanEssentials);
ComponentSystem* loadHopsanModelFile(char* xmlStr, HopsanEssentials* pHopsanEssentials);
void loadHopsanParameterFile(const HString filePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem);

}

#endif

