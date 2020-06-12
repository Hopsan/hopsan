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

//!
//! @file   HopsanEssentials.h
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#ifndef HopsanEssentials_H
#define HopsanEssentials_H

#define HOPSAN_BUILTIN_TYPENAME_DUMMYCOMPONENT "DummyComponent"
#define HOPSAN_BUILTIN_TYPENAME_MISSINGCOMPONENT "MissingComponent"
#define HOPSAN_BUILTIN_TYPENAME_SUBSYSTEM "Subsystem"
#define HOPSAN_BUILTIN_TYPENAME_CONDITIONALSUBSYSTEM "ConditionalSubsystem"

#include "Node.h"
#include "Component.h"
#include "ComponentSystem.h"
#include <vector>

namespace hopsan {

//Forward Declaration
class LoadExternal;
class HopsanCoreMessageHandler;
class QuantityRegister;

//! @brief This class gives access to HopsanCore for model and externalLib loading as well as component creation and simulation.
class HOPSANCORE_DLLAPI HopsanEssentials
{
private:
    NodeFactory* mpNodeFactory;
    ComponentFactory* mpComponentFactory;
    HopsanCoreMessageHandler* mpMessageHandler;
    LoadExternal* mpExternalLoader;
    SimulationHandler mSimulationHandler;
    QuantityRegister* mpQuantityRegister;
    static size_t mInstanceCounter;

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

    // Messages and log
    HopsanCoreMessageHandler* getCoreMessageHandler();
    void getMessage(HString &rMessage, HString &rType, HString &rTag);
    size_t checkMessage();
    size_t getNumInfoMessages() const;
    size_t getNumWarningMessages() const;
    size_t getNumErrorMessages() const;
    size_t getNumFatalMessages() const;
    size_t getNumDebugMessages() const;

    bool openCoreLogFile(const char* absoluteFilePath);

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


void HOPSANCORE_DLLAPI addCoreLogMessage(const char* message);
inline void addCoreLogMessage(const HString& message)
{
    addCoreLogMessage(message.c_str());
}

// This one is for backwards compatibility, due to name change
inline void addLogMess(const char* message)
{
    addCoreLogMessage(message);
}
// This one is for backwards compatibility, due to name change
inline void addLogMess(const HString& message)
{
    addCoreLogMessage(message.c_str());
}

}
#endif // HopsanEssentials_H
