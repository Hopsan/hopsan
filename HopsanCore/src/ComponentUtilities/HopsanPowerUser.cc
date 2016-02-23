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
