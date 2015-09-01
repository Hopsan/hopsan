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
//! @file   HopsanEssentials.h
//! @author <peter.nordin@liu.se>
//! @date   2010-02-19
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#ifndef HopsanEssentials_H
#define HopsanEssentials_H

#define DUMMYTYPENAME "DummyComponent"
#define MISSINGTYPENAME "MissingComponent"
#define SUBSYSTEMTYPENAME "Subsystem"
#define CONDITIONALTYPENAME "ConditionalSubsystem"
#define CPPTYPENAME "CppComponent"
#define MODELICATYPENAME "ModelicaComponent"

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
    const char *getCoreVersion() const;
    const char *getCoreBuildTime() const;
    const char *getCoreCompiler() const;
    bool isCore64Bit() const;
    bool isCoreDebugCompiled() const;

    // Component creation
    Component* createComponent(const HString &rTypeName);
    ComponentSystem* createComponentSystem();
    ConditionalComponentSystem* createConditionalComponentSystem();
    void removeComponent(Component *pComponent);
    void removeNode(Node *pNode);
    bool hasComponent(const HString &rType) const;
    bool reserveComponentTypeName(const HString &rTypeName);
    const std::vector<HString> getRegisteredComponentTypes() const;

    // Node creation
    Node* createNode(const HString &rNodeType);
    const std::vector<HString> getRegisteredNodeTypes() const;

    // Quantities
    bool haveQuantity(const HString &rQuantity) const;

    // Messages
    HopsanCoreMessageHandler* getCoreMessageHandler();
    void getMessage(HString &rMessage, HString &rType, HString &rTag);
    size_t checkMessage();
    size_t getNumInfoMessages() const;
    size_t getNumWarningMessages() const;
    size_t getNumErrorMessages() const;
    size_t getNumFatalMessages() const;
    size_t getNumDebugMessages() const;

    // External libraries
    bool loadExternalComponentLib(const char* path);
    bool unLoadExternalComponentLib(const char* path);
    void getExternalComponentLibNames(std::vector<HString> &rLibNames);
    void getExternalLibraryContents(const char* libPath, std::vector<HString> &rComponents, std::vector<HString> &rNodes);
    void getLibPathForComponentType(const HString &rTypeName, HString &rLibPath);

    // Loading HMF models
    ComponentSystem* loadHMFModelFile(const char* filePath, double &rStartTime, double &rStopTime);
    ComponentSystem* loadHMFModel(const std::vector<unsigned char> xmlVector);
    ComponentSystem* loadHMFModel(const char* xmlString, double &rStartTime, double &rStopTime);

    // Running simulation
    SimulationHandler *getSimulationHandler();
};

void openLogFile();
void closeLogFile();
void addLogMess(const char* message);
}
#endif // HopsanEssentials_H
