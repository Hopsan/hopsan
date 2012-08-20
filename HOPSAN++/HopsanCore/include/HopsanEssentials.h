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
//! @file   HopsanEssentials.h
//! @author <peter.nordin@liu.se>
//! @date   2010-02-19
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#ifndef HopsanEssentials_H
#define HopsanEssentials_H

#include "Node.h"
#include "Component.h"
#include "ComponentSystem.h"
#include <vector>
#include <fstream>

namespace hopsan {
extern "C" {

//Forward Declaration
class LoadExternal;
class HopsanCoreMessageHandler;

//! @brief This class gives access to HopsanCore for model and externalLib loading as well as component creation and simulation.
class DLLIMPORTEXPORT HopsanEssentials
{
private:
    NodeFactory* mpNodeFactory;
    ComponentFactory* mpComponentFactory;
    HopsanCoreMessageHandler* mpMessageHandler;
    LoadExternal* mpExternalLoader;
    SimulationHandler mSimulationHandler;

public:
    HopsanEssentials();
    ~HopsanEssentials();

    // Version info
    std::string getCoreVersion();

    // Component creation
    Component* createComponent(const std::string &rTypeName);
    ComponentSystem* createComponentSystem();
    bool hasComponent(const std::string type);
    bool reserveComponentTypeName(const std::string typeName);

    // Node creation
    Node* createNode(const NodeTypeT &rNodeType);

    // Messages
    HopsanCoreMessageHandler* getCoreMessageHandler();
    void getMessage(std::string &rMessage, std::string &rType, std::string &rTag);
    size_t checkMessage();

    // External libraries
    bool loadExternalComponentLib(const std::string path);
    bool unLoadExternalComponentLib(const std::string path);
    void getExternalComponentLibNames(std::vector<std::string> &rLibNames);

    // Loading HMF models
    ComponentSystem* loadHMFModel(const std::string filePath, double &rStartTime, double &rStopTime);
    ComponentSystem* loadHMFModel(const std::vector<unsigned char> xmlVector);

    // Running simulation
    SimulationHandler *getSimulationHandler();
};
}

void addLogMess(const std::string log);
static std::ofstream hopsanLogFile;
}
#endif // HopsanEssentials_H
