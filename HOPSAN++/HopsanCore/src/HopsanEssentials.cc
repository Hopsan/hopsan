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
//! @file   HopsanEssentials.cc
//! @author <peter.nordin@liu.se>
//! @date   2010-02-19
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#include "HopsanEssentials.h"
#include "CoreUtilities/StringUtilities.h"
#include "Nodes.h"
#include "version.h"
#include "CoreUtilities/ClassFactoryStatusCheck.hpp"
#include "Components/DummyComponent.hpp"
#include "CoreUtilities/HmfLoader.h"
#include "CoreUtilities/LoadExternal.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>


#ifdef BUILTINDEFAULTCOMPONENTLIB
#include "defaultComponentLibraryInternal.h"
#endif

using namespace std;
using namespace hopsan;

//! @brief HopsanEssentials Constructor
HopsanEssentials::HopsanEssentials()
{
    // Create Factories and handlers
    mpNodeFactory = new NodeFactory;
    mpComponentFactory = new ComponentFactory;
    mpMessageHandler = new HopsanCoreMessageHandler;
    mpExternalLoader = new LoadExternal(mpComponentFactory, mpNodeFactory, mpMessageHandler);

    // Make sure that internal Nodes and Components register
    register_default_nodes(mpNodeFactory);
    mpComponentFactory->registerCreatorFunction("DummyComponent", DummyComponent::Creator);
    mpComponentFactory->registerCreatorFunction("MissingComponent", DummyComponent::Creator);
    mpComponentFactory->registerCreatorFunction("Subsystem", ComponentSystem::Creator);
    mpComponentFactory->registerCreatorFunction("CppComponent", DummyComponent::Creator);
    mpComponentFactory->registerCreatorFunction("ModelicaComponent", DummyComponent::Creator);
#ifdef BUILTINDEFAULTCOMPONENTLIB
    register_default_components(mpComponentFactory);
#endif

    // Check for register errors and status
    checkClassFactoryStatus(mpComponentFactory, mpMessageHandler);
    checkClassFactoryStatus(mpNodeFactory, mpMessageHandler);

    // Clear factory status
    mpComponentFactory->clearRegisterStatus();
    mpNodeFactory->clearRegisterStatus();


    // Do some other stuff
    mpMessageHandler->addInfoMessage("HopsanCore, Version: " + string(HOPSANCOREVERSION));

    openLogFile();
    addLogMess("This file logs the actions done by HopsanCore,\nto trace a program crash one can see what was the last logged action.\nLook at the last rows in this file.\n\n\n");
}

//! @brief HopsanEssentials Destructor
HopsanEssentials::~HopsanEssentials()
{
    //Clear the factories
    //! @todo need to make sure that every one has destoyed all components/nodes before we unregister them, it probably cant be done from inside here
    mpNodeFactory->clearFactory();
    mpComponentFactory->clearFactory();
    closeLogFile();

    delete mpNodeFactory;
    delete mpComponentFactory;

    // Delete the messsage handler
    delete mpMessageHandler;
}

//! Returns the hopsan core version as a string
std::string HopsanEssentials::getCoreVersion()
{
    return HOPSANCOREVERSION;
}

//! Creates a component with the specified key-value and returns a pointer to this component.
//! @param [in] rString The
Component* HopsanEssentials::createComponent(const string &rTypeName)
{
    addLogMess(rTypeName + "::createComponent");
    Component* pComp = mpComponentFactory->createInstance(rTypeName.c_str());
    if (pComp)
    {
        pComp->mpHopsanEssentials = this;
        pComp->mpMessageHandler = mpMessageHandler; //!< @todo maybe it should only be in HopsanEssentials but then the core message handler will be accesible in main program also (maybe not a big deal)
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
//! @param [in] type The typename to check
//! @returns True or False depending on if type exist
bool HopsanEssentials::hasComponent(const string type)
{
    return mpComponentFactory->hasKey(type.c_str());
}

//! @brief Reserves a component TypeName in the component factory map
//! @param [in] typeName The TypeName to reserve
bool HopsanEssentials::reserveComponentTypeName(const std::string typeName)
{
    return mpComponentFactory->reserveKey(typeName);
}

//! @brief Returns a vector containing all registered component types
std::vector<string> HopsanEssentials::getRegisteredComponentTypes() const
{
    return mpComponentFactory->getRegisteredKeys();
}


//! @brief Creates a ComponentSystem
//! @returns A pointer to the ComponentSystem created
ComponentSystem* HopsanEssentials::createComponentSystem()
{
    return static_cast<ComponentSystem*>(createComponent("Subsystem"));
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
Node* HopsanEssentials::createNode(const std::string &rNodeType)
{
    Node *pNode = mpNodeFactory->createInstance(rNodeType.c_str());
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
std::vector<string> HopsanEssentials::getRegisteredNodeTypes() const
{
    return mpNodeFactory->getRegisteredKeys();
}

//! @brief Returns a pointer to the core message handler, do NOT use this function to get messages
HopsanCoreMessageHandler *HopsanEssentials::getCoreMessageHandler()
{
    return mpMessageHandler;
}

//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [in,out] rStartTime A reference to the starttime variable
//! @param [in,out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* HopsanEssentials::loadHMFModel(const string filePath, double &rStartTime, double &rStopTime)
{
    addLogMess("HopsanEssentials::loadHMFModel()");
    return loadHopsanModelFile(filePath, this, rStartTime, rStopTime);
}

ComponentSystem* HopsanEssentials::loadHMFModel(const std::vector<unsigned char> xmlVector)
{
    return loadHopsanModelFile(xmlVector, this);
}

ComponentSystem* HopsanEssentials::loadHMFModel(const std::string xmlString)
{
    return loadHopsanModelFileFromStdString(xmlString, this);
}

SimulationHandler *HopsanEssentials::getSimulationHandler()
{
    return &mSimulationHandler;
}

//! @brief Get the message waiting on the message queue
//! @param [out] rMessage A reference to the message string
//! @param [out] rType A reference to the message type string
//! @param [out] rTag A reference to the message type Tag
void HopsanEssentials::getMessage(char **message, char **type, char **tag)
{
    //! @todo Utility function
    HopsanCoreMessage msg = mpMessageHandler->getMessage();
//    *message = (char *)malloc((strlen(msg.mMessage.c_str())+1)*sizeof(char));
//    strcpy(*message, msg.mMessage.c_str());
//    *tag = (char *)malloc((strlen(msg.mTag.c_str())+1)*sizeof(char));
//    strcpy(*tag, msg.mTag.c_str());
    copyString(message, msg.mMessage);
    copyString(tag, msg.mTag);

    switch (msg.mType)
    {
    case HopsanCoreMessage::Fatal:
        copyString(type, "fatal");
        //*type = "fatal";
        break;
    case HopsanCoreMessage::Error:
        *type = "error";
        break;
    case HopsanCoreMessage::Warning:
        *type = "warning";
        break;
    case HopsanCoreMessage::Info:
        *type = "info";
        break;
    case HopsanCoreMessage::Debug:
        *type = "debug";
        break;
    }
}

//! @brief Check if there are any messages waiting in the queue
//! @returns The number of waiting messages
size_t HopsanEssentials::checkMessage()
{
    return mpMessageHandler->getNumWaitingMessages();
}

//! @brief Loads an external component library
//! @param [in] path The path to the library DLL or SO file
//! @returns True if loaded sucessfully, otherwise false
bool HopsanEssentials::loadExternalComponentLib(const string path)
{
    return mpExternalLoader->load(path);
}

//! @brief Unloads an external component library
//! @param [in] path The path to the library DLL or SO file to unload
//! @returns True if unloaded sucessfully, otherwise false
bool HopsanEssentials::unLoadExternalComponentLib(const std::string path)
{
    return mpExternalLoader->unLoad(path);
}

//! @brief Get the libNames of the currently loaded libs (the names compiled into libs)
//! @param [out] rLibNames A reference to the vector that will contain the lib names
void HopsanEssentials::getExternalComponentLibNames(std::vector<std::string> &rLibNames)
{
    mpExternalLoader->getLoadedLibNames(rLibNames);
}


//! @brief Get the contents (components and nodes) registered by an external library
//! @param [in] libpath Path to the external library
//! @param [out] rComponents A reference to the vector that will contain the component names
//! @param [out] rNodes A reference to the vector that will contain the node names
void HopsanEssentials::getExternalLibraryContents(const std::string libpath, std::vector<std::string> &rComponents, std::vector<std::string> &rNodes)
{
    mpExternalLoader->getLibContents(libpath, rComponents, rNodes);
}


static std::ofstream hopsanLogFile;


void hopsan::openLogFile()
{
#ifdef MAINCORE
    hopsanLogFile.open("hopsan_logfile.txt");
#endif
}


void hopsan::closeLogFile()
{
#ifdef MAINCORE
    hopsanLogFile.close();
#endif
}

//! @brief Adds a message to the HopsanCore runtime log
void hopsan::addLogMess(const std::string log)
{
#ifdef MAINCORE
    if(hopsanLogFile.good())
    {
        hopsanLogFile << log << "\n";
    }
#endif
}
