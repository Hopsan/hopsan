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

//$Id$

#ifndef HOPSANPOWERUSER_H
#define HOPSANPOWERUSER_H

#include "HopsanEssentials.h"
#include "win32dll.h"

namespace hopsan {

Component DLLIMPORTEXPORT *createSafeComponent(ComponentSystem *pSystem, const HString &rType);
bool DLLIMPORTEXPORT smartConnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2);
bool DLLIMPORTEXPORT smartDisconnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2);


//! @brief Help function to safely get the internal parameter data pointer from a subcomponent, the type needs to be known
//! If parameter or component NULL, then error message instead of crash
//! @note circumvents the ordinary parameter system, use only if you know what you are doing
//! @note It will only work for Constants not input/output variables (then you will get pointer to start value instead)
//! @ingroup ComponentPowerAuthorFunctions
//! @returns A pointer to the parameter or a dummy parameter (to avoid crash on further use)
template<typename T>
T* getSafeConstantDataPtr(ComponentSystem *pSystem, Component *pComp, const HString &rConstantName)
{
    T* pTmp = 0;
    HString compType = "NULL";

    // First handle if component pointer is null
    if (pComp != 0)
    {
        pTmp = static_cast<T*>(pComp->getParameterDataPtr(rConstantName));
        compType = pComp->getTypeName();
    }

    // Now check if we found the constant, if not return dummy, error message and stop simulation
    if (pTmp == 0)
    {
        pSystem->addErrorMessage("Could not get constant data ptr from subcomponent: " + compType);
        pTmp = new T; //OK! there is a small memory leak here, but it only happens when you make a mistake, ant then you restart and recompile anyway
        pSystem->stopSimulation();
    }
    return pTmp;
}


}
#endif // HOPSANPOWERUSER_H
