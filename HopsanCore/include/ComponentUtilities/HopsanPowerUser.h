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

//$Id$

#ifndef HOPSANPOWERUSER_H
#define HOPSANPOWERUSER_H

#include "HopsanEssentials.h"
#include "win32dll.h"

namespace hopsan {

Component HOPSANCORE_DLLAPI *createSafeComponent(ComponentSystem *pSystem, const HString &rType);
bool HOPSANCORE_DLLAPI smartConnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2);
bool HOPSANCORE_DLLAPI smartDisconnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2);


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
