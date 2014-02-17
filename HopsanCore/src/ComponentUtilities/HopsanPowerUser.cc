#include "ComponentUtilities/HopsanPowerUser.h"

using namespace hopsan;

//! @brief Helpfunction to create components and abort safely if that fails
//! @param [in] pSystem A pointer to the system in which to create the component
//! @param [in] rType A string with the unique typename of the component to create
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

//! @brief Helpfunction that only call connect if the ports are not already connected
//! @param [in] pSystem The system to handle the connection
//! @param [in] pPort1 The first port to connect
//! @param [in] pPort2 The other port to connect
//! @returns true if connection was Ok, else false
bool hopsan::smartConnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2)
{
    if (!pPort1->isConnectedTo(pPort2))
    {
        return pSystem->connect(pPort1, pPort2);
    }
    return true;
}

//! @brief Helpfunction that only call disconnect if the ports are connected
//! @param [in] pSystem The system to handle the disconnection
//! @param [in] pPort1 The first port to disconnect
//! @param [in] pPort2 The other port to disconnect
//! @returns true if disconnection was Ok, else false
bool hopsan::smartDisconnect(ComponentSystem *pSystem, Port *pPort1, Port *pPort2)
{
    if (pPort1->isConnectedTo(pPort2))
    {
        return pSystem->disconnect(pPort1, pPort2);
    }
    return true;
}
