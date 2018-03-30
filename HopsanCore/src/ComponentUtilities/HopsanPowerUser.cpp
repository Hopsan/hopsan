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

#include "ComponentUtilities/HopsanPowerUser.h"

using namespace hopsan;

//! @brief Help function to create components and abort safely if that fails
//! @param [in] pSystem A pointer to the system in which to create the component
//! @param [in] rType A string with the unique type name of the component to create
//! @ingroup ComponentPowerAuthorFunctions
//! @returns Pointer to created component or dummy
Component* hopsan::createSafeComponent(ComponentSystem *pSystem, const HString &rType)
{
    Component* pComp = pSystem->getHopsanEssentials()->createComponent(rType);
    if (pComp == 0)
    {
        pSystem->addErrorMessage("Could not create subcomponent: " + rType + " returning DummyComponent");
        pComp = pSystem->getHopsanEssentials()->createComponent("DummyComponent");
        pSystem->stopSimulation();
    }
    return pComp;
}

//! @brief Help function that only call connect if the ports are not already connected to each other
//! @param [in] pSystem The system to handle the connection
//! @param [in] pPort1 The first port to connect
//! @param [in] pPort2 The other port to connect
//! @ingroup ComponentPowerAuthorFunctions
//! @returns true if connection was OK, else false
bool hopsan::smartConnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2)
{
    // Fail if any pointer is nullptr
    if (!pSystem)
    {
        return false;
    }
    if (!(pPort1 && pPort2))
    {
        pSystem->addErrorMessage("In smartConnect(): pPort1 or pPort2 is 0");
        return false;
    }

    if (!pPort1->isConnectedTo(pPort2))
    {
        return pSystem->connect(pPort1, pPort2);
    }
    return true;
}

//! @brief Help function that only call disconnect if the ports are connected to each other
//! @param [in] pSystem The system to handle the disconnection
//! @param [in] pPort1 The first port to disconnect
//! @param [in] pPort2 The other port to disconnect
//! @ingroup ComponentPowerAuthorFunctions
//! @returns true if disconnection was OK, else false
bool hopsan::smartDisconnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2)
{
    // Fail if any pointer is nullptr
    if (!pSystem)
    {
        return false;
    }
    if (!(pPort1 && pPort2))
    {
        pSystem->addErrorMessage("In smartConnect(): pPort1 or pPort2 is 0");
        return false;
    }

    if (pPort1->isConnectedTo(pPort2))
    {
        return pSystem->disconnect(pPort1, pPort2);
    }
    return true;
}
