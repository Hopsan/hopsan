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

namespace hopsan {

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
    const char *getCoreVersion();

    // Component creation
    Component* createComponent(const char* typeName);
    ComponentSystem* createComponentSystem();
    void removeComponent(Component *pComponent);
    void removeNode(Node *pNode);
    bool hasComponent(const char* type);
    bool reserveComponentTypeName(const char* typeName);
    const std::vector<HString> getRegisteredComponentTypes() const;

    // Node creation
    Node* createNode(const char* nodeType);
    const std::vector<HString> getRegisteredNodeTypes() const;

    // Messages
    HopsanCoreMessageHandler* getCoreMessageHandler();
    void getMessage(HString &rMessage, HString &rType, HString &rTag);
    size_t checkMessage();

    // External libraries
    bool loadExternalComponentLib(const char* path);
    bool unLoadExternalComponentLib(const char* path);
    void getExternalComponentLibNames(std::vector<HString> &rLibNames);
    void getExternalLibraryContents(const char* libPath, std::vector<HString> &rComponents, std::vector<HString> &rNodes);

    // Loading HMF models
    ComponentSystem* loadHMFModel(const char* filePath, double &rStartTime, double &rStopTime);
    ComponentSystem* loadHMFModel(const std::vector<unsigned char> xmlVector);
    ComponentSystem* loadHMFModel(const char* xmlString);

    // Running simulation
    SimulationHandler *getSimulationHandler();
};

void openLogFile();
void closeLogFile();
void addLogMess(const char* message);
}
#endif // HopsanEssentials_H
