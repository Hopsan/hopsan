#ifndef HOPSANPOWERUSER_H
#define HOPSANPOWERUSER_H

#include "HopsanEssentials.h"
#include "win32dll.h"

namespace hopsan {

Component* DLLIMPORTEXPORT createSafeComponent(ComponentSystem *pSystem, const HString &rType);
bool DLLIMPORTEXPORT smartConnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2);
bool DLLIMPORTEXPORT smartDisconnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2);


//! @brief Helpfunction to safely get the internal parameter data ptr from a subcomponent, the type needs to be known
//! If parameter or component NULL, then error message instead of crash
//! @note circumvents the ordinary parameter system, use only if you know what you are doing
//! @note It will only work for Constants not input/output variables (then you will get ptr to startvalue instead)
//! @returns A pointer to the parameter or a dummy parameter (to avoid crash on further use)
template<typename T>
T* getSafeConstantDataPtr(ComponentSystem *pSystem, Component *pComp, const HString &rConstantName)
{
    T* pTmp = 0;
    HString compType = "NULL";

    // First handle if component ptr is null
    if (pComp != 0)
    {
        pTmp = static_cast<T*>(pComp->getParameterDataPtr(rConstantName));
        compType = pComp->getTypeName();
    }

    // Now check if we found the constant, if not return dummy, error message and stop simulation
    if (pTmp == 0)
    {
        pSystem->addErrorMessage("Could not get constant data ptr from subcomponent: " + compType);
        pTmp = new T; //Ok! there is a small memory leak here, but it only happens when you make a mistake, ant then you restart and recompile anyway
        pSystem->stopSimulation();
    }
    return pTmp;
}


}
#endif // HOPSANPOWERUSER_H
