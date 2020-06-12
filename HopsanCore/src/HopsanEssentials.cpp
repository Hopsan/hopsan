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
//! @file   HopsanEssentials.cpp
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#include "HopsanEssentials.h"
#include "CoreUtilities/StringUtilities.h"
#include "Nodes.h"
#include "HopsanCoreVersion.h"
#include "CoreUtilities/ClassFactoryStatusCheck.hpp"
#include "Components/DummyComponent.hpp"
#include "CoreUtilities/HmfLoader.h"
#include "CoreUtilities/LoadExternal.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Quantities.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>


#ifdef HOPSAN_INTERNALDEFAULTCOMPONENTS
#include "defaultComponentLibraryInternal.h"
#endif

#if defined(HOPSAN_INTERNAL_EXTRACOMPONENTS)
namespace hopsan {
    void register_extra_components(hopsan::ComponentFactory* pComponentFactory);
}
#endif

using namespace std;
using namespace hopsan;

namespace {

#ifdef HOPSANCORE_WRITELOG
static std::ofstream gCoreLogFile;
#endif

//! @brief Closes the HopsanCore runtime log when refcounter reaches 0
void closeCoreLogFile()
{
#ifdef HOPSANCORE_WRITELOG
    if (gCoreLogFile.is_open()) {
        gCoreLogFile.close();
    }
#endif
}

}

QuantityRegister *hopsan::gpInternalCoreQuantityRegister=0; // Do not use this pointer outside of HopsanCore
size_t HopsanEssentials::mInstanceCounter = 0;

//! @brief HopsanEssentials Constructor
HopsanEssentials::HopsanEssentials()
{
    mInstanceCounter++;

    // Create Factories and handlers
    mpNodeFactory = new NodeFactory;
    mpComponentFactory = new ComponentFactory;
    mpMessageHandler = new HopsanCoreMessageHandler;
    if (mInstanceCounter==1)
    {
        mpQuantityRegister = new QuantityRegister;
        gpInternalCoreQuantityRegister = mpQuantityRegister;
    }
    else
    {
        mpQuantityRegister = gpInternalCoreQuantityRegister;
    }

    mpExternalLoader = new LoadExternal(mpComponentFactory, mpNodeFactory, mpMessageHandler);

    // Make sure that internal Nodes and Components register
    register_default_nodes(mpNodeFactory);
    mpComponentFactory->registerCreatorFunction(HOPSAN_BUILTIN_TYPENAME_DUMMYCOMPONENT, DummyComponent::Creator);
    mpComponentFactory->registerCreatorFunction(HOPSAN_BUILTIN_TYPENAME_MISSINGCOMPONENT, DummyComponent::Creator);
    mpComponentFactory->registerCreatorFunction(HOPSAN_BUILTIN_TYPENAME_SUBSYSTEM, ComponentSystem::Creator);
    mpComponentFactory->registerCreatorFunction(HOPSAN_BUILTIN_TYPENAME_CONDITIONALSUBSYSTEM, ConditionalComponentSystem::Creator);

#ifdef HOPSAN_INTERNALDEFAULTCOMPONENTS
    register_default_components(mpComponentFactory);
#endif
#if defined(HOPSAN_INTERNAL_EXTRACOMPONENTS)
    register_extra_components(mpComponentFactory);
#endif

    // Check for register errors and status
    checkClassFactoryStatus(mpComponentFactory, mpMessageHandler);
    checkClassFactoryStatus(mpNodeFactory, mpMessageHandler);

    // Clear factory status
    mpComponentFactory->clearRegisterStatus();
    mpNodeFactory->clearRegisterStatus();

    // Do some other stuff
    HString debugtext;
    if (isCoreDebugCompiled())
    {
        debugtext=" (Compiled in debug mode)";
    }
    if (isCore64Bit())
    {
        mpMessageHandler->addInfoMessage("HopsanCore 64-bit, Version: " + HString(HOPSANCOREVERSION)+debugtext);
    }
    else
    {
        mpMessageHandler->addInfoMessage("HopsanCore 32-bit, Version: " + HString(HOPSANCOREVERSION)+debugtext);
    }
}

//! @brief HopsanEssentials Destructor
HopsanEssentials::~HopsanEssentials()
{
    //Clear the factories
    //! @todo need to make sure that every one has destroyed all components/nodes before we unregister them, it probably cant be done from inside here
    mpNodeFactory->clearFactory();
    mpComponentFactory->clearFactory();

    delete mpNodeFactory;
    delete mpComponentFactory;

    // Delete the message handler
    delete mpMessageHandler;

    if (mInstanceCounter==1)
    {
        delete mpQuantityRegister;
        gpInternalCoreQuantityRegister = 0;
        closeCoreLogFile();
    }

    mInstanceCounter--;
}

//! @brief Opens the HopsanCore runtime log file, if not already opened
//! @param[in] absoluteFilePath The file path of the log file
//! @returns True if the file could be opened, else false
bool HopsanEssentials::openCoreLogFile(const char* absoluteFilePath)
{
    bool opened = false;
#ifdef HOPSANCORE_WRITELOG
    if (!gCoreLogFile.is_open()) {
        gCoreLogFile.open(absoluteFilePath);
        opened = gCoreLogFile.is_open();
    }
#endif
    if (opened) {
        addCoreLogMessage("This file logs the actions made by HopsanCore.\nTo trace a program crash you can look at the last line in this file to see what the last logged action was.\n\n\n");
    }
    return opened;
}

//! @brief Returns the HopsanCore version as a string
const char *HopsanEssentials::getCoreVersion() const
{
    return HOPSANCOREVERSION;
}

//! @brief Returns the HopsanCore build date and time
const char *HopsanEssentials::getCoreBuildTime() const
{
    return __DATE__ " " __TIME__;
}

//! @brief Get compiler info from core
const char *HopsanEssentials::getCoreCompiler() const
{
    return HOPSANCOMPILEDWITH;
}

//! @brief Check if core is compiled 64-bit
//! @returns Returns true if core is compiled as 64-bit else returns false
bool HopsanEssentials::isCore64Bit() const
{
#ifdef HOPSANCOMPILED64BIT
    return true;
#else
    return false;
#endif
}

bool HopsanEssentials::isCoreDebugCompiled() const
{
#ifdef HOPSAN_BUILD_TYPE_DEBUG
    return true;
#else
    return false;
#endif
}

//! @brief Creates a component with the specified key-value and returns a pointer to this component.
//! @param [in] rTypeName The unique type identifier of the component to create
Component* HopsanEssentials::createComponent(const HString &rTypeName)
{
    addCoreLogMessage(rTypeName+"::createComponent");
    Component* pComp = mpComponentFactory->createInstance(rTypeName);
    if (pComp)
    {
        pComp->mpHopsanEssentials = this;
        pComp->mpMessageHandler = mpMessageHandler; //!< @todo maybe it should only be in HopsanEssentials but then the core message handler will be accessible in main program also (maybe not a big deal)
        pComp->setTypeName(rTypeName);
        pComp->setName(rTypeName);
        pComp->configure();
    }
    else
    {
        checkClassFactoryStatus(mpComponentFactory, mpMessageHandler);
        mpComponentFactory->clearRegisterStatus();
    }
    return pComp;
}

//! @brief Check if a component with given typename exist in the ComponentFactory
//! @param [in] rType The typename to check
//! @returns True or False depending on if type exist
bool HopsanEssentials::hasComponent(const HString &rType) const
{
    return mpComponentFactory->hasKey(rType);
}

//! @brief Reserves a component TypeName in the component factory map
//! @param [in] rTypeName The TypeName to reserve
bool HopsanEssentials::reserveComponentTypeName(const HString &rTypeName)
{
    return mpComponentFactory->reserveKey(rTypeName);
}

//! @brief Returns a vector containing all registered component types
const std::vector<HString> HopsanEssentials::getRegisteredComponentTypes() const
{
    return mpComponentFactory->getRegisteredKeys();
}


//! @brief Creates a ComponentSystem
//! @returns A pointer to the ComponentSystem created
ComponentSystem* HopsanEssentials::createComponentSystem()
{
    return static_cast<ComponentSystem*>(createComponent("Subsystem"));
}


//! @brief Creates a ConditionalComponentSystem
//! @returns A pointer to the ComponentSystem created
ConditionalComponentSystem *HopsanEssentials::createConditionalComponentSystem()
{
    return static_cast<ConditionalComponentSystem*>(createComponent("ConditionalSubsystem"));
}

void HopsanEssentials::removeComponent(Component *pComponent)
{
    pComponent->deconfigure();
    delete pComponent; //! @todo can I really delete here or do I need to use the factory for external components
}

void HopsanEssentials::removeNode(Node *pNode)
{
    delete pNode;
}

//! @brief Creates a Node of given node type
//! @param [in] rNodeType The type of node to create
//! @returns A pointer to the created node
Node* HopsanEssentials::createNode(const HString &rNodeType)
{
    Node *pNode = mpNodeFactory->createInstance(rNodeType);
    if (pNode)
    {
        pNode->mNodeType = rNodeType;
    }
    else
    {
        checkClassFactoryStatus(mpNodeFactory, mpMessageHandler);
        mpNodeFactory->clearRegisterStatus();
    }
    return pNode;
}

//! @brief Returns a vector containing all registered node types
const std::vector<HString> HopsanEssentials::getRegisteredNodeTypes() const
{
    return mpNodeFactory->getRegisteredKeys();
}

//! @brief Check if a quantity name is registered
//! @param[in] rQuantity The name of the quantity
//! @returns true if the quantity is registered, else false
bool HopsanEssentials::haveQuantity(const HString &rQuantity) const
{
    return mpQuantityRegister->haveQuantity(rQuantity);
}

//! @brief Returns a pointer to the core message handler, do NOT use this function to get messages
HopsanCoreMessageHandler *HopsanEssentials::getCoreMessageHandler()
{
    return mpMessageHandler;
}

//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the root system of the loaded model
ComponentSystem* HopsanEssentials::loadHMFModelFile(const char *filePath, double &rStartTime, double &rStopTime)
{
    return loadHopsanModelFile(filePath, this, rStartTime, rStopTime);
}

ComponentSystem* HopsanEssentials::loadHMFModel(const std::vector<unsigned char> xmlVector)
{
    return loadHopsanModel(xmlVector, this);
}

//! @brief This function is used to load a HMF model from a string
//! @param [in] xmlString The model xml string
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the root system of the loaded model
ComponentSystem* HopsanEssentials::loadHMFModel(const char *xmlString, double &rStartTime, double &rStopTime)
{
    return loadHopsanModel(xmlString, this, rStartTime, rStopTime);
}

SimulationHandler *HopsanEssentials::getSimulationHandler()
{
    return &mSimulationHandler;
}

//! @brief Get the message waiting on the message queue
//! @param [out] rMessage A reference to the message string
//! @param [out] rType A reference to the message type string
//! @param [out] rTag A reference to the message type Tag
void HopsanEssentials::getMessage(HString &rMessage, HString &rType, HString &rTag)
{
    mpMessageHandler->getMessage(rMessage, rType, rTag);
}

//! @brief Check if there are any messages waiting in the queue
//! @returns The number of waiting messages
size_t HopsanEssentials::checkMessage()
{
    return mpMessageHandler->getNumWaitingMessages();
}

//! @brief Returns the number of waiting info messages on the message queue
size_t HopsanEssentials::getNumInfoMessages() const
{
    return mpMessageHandler->getNumInfoMessages();
}

//! @brief Returns the number of waiting warning messages on the message queue
size_t HopsanEssentials::getNumWarningMessages() const
{
    return mpMessageHandler->getNumWarningMessages();
}

//! @brief Returns the number of waiting error messages on the message queue
size_t HopsanEssentials::getNumErrorMessages() const
{
    return mpMessageHandler->getNumErrorMessages();
}

//! @brief Returns the number of waiting fatal messages on the message queue
size_t HopsanEssentials::getNumFatalMessages() const
{
    return mpMessageHandler->getNumFatalMessages();
}

//! @brief Returns the number of waiting debug messages on the message queue
size_t HopsanEssentials::getNumDebugMessages() const
{
    return mpMessageHandler->getNumDebugMessages();
}

//! @brief Loads an external component library
//! @param [in] path The path to the library DLL or SO file
//! @returns True if loaded successfully, otherwise false
bool HopsanEssentials::loadExternalComponentLib(const char *path)
{
    return mpExternalLoader->load(path);
}

//! @brief Unloads an external component library
//! @param [in] path The path to the library DLL or SO file to unload
//! @returns True if unloaded successfully, otherwise false
bool HopsanEssentials::unLoadExternalComponentLib(const char *path)
{
    return mpExternalLoader->unLoad(path);
}

//! @brief Get the libNames of the currently loaded libs (the names compiled into libs)
//! @param [out] rLibNames A reference to the vector that will contain the lib names
void HopsanEssentials::getExternalComponentLibNames(std::vector<HString> &rLibNames)
{
    mpExternalLoader->getLoadedLibNames(rLibNames);
}

//! @brief Returns the path to the library file from where specified component is loaded
//! @param rTypeName Type name of component
//! @param rLibPath Reference string where path is stored
void HopsanEssentials::getLibPathForComponentType(const HString &rTypeName, HString &rLibPath)
{
    mpExternalLoader->getLibPathByTypeName(rTypeName, rLibPath);
}

//! @brief Get the contents (components and nodes) registered by an external library
//! @param [in] libPath Path to the external library
//! @param [out] rComponents A reference to the vector that will contain the component names
//! @param [out] rNodes A reference to the vector that will contain the node names
void HopsanEssentials::getExternalLibraryContents(const char *libPath, std::vector<HString> &rComponents, std::vector<HString> &rNodes)
{
    mpExternalLoader->getLibContents(libPath, rComponents, rNodes);
}


//! @brief Adds a message to the HopsanCore runtime log
//! @param[in] message The message to write to the log file
void hopsan::addCoreLogMessage(const char *message)
{
#ifdef HOPSANCORE_WRITELOG
    if(gCoreLogFile.good()) {
        gCoreLogFile << message << std::endl;
    }
#endif
}
