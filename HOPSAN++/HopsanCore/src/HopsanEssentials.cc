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
#include "Nodes.h"
#include "version.h"
#include "CoreUtilities/ClassFactoryStatusCheck.hpp"
#include "Components/DummyComponent.hpp"
#include "CoreUtilities/HmfLoader.h"
#include <string>

#ifdef BUILTINDEFAULTCOMPONENTLIB
#include "defaultComponentLibraryInternal.h"
#endif

using namespace std;
using namespace hopsan;

//Set the stacic start values
bool HopsanEssentials::mHasInstance = false;
HopsanEssentials* HopsanEssentials::mpInstance = 0;

//! @brief This function initializes the HopsanEssential singleton object
void HopsanEssentials::initialize()
{
    //Make sure that internal Nodes and Components register
    register_nodes(mpNodeFactory);
    mpComponentFactory->registerCreatorFunction("MissingComponent", DummyComponent::Creator);
    mpComponentFactory->registerCreatorFunction("Subsystem", ComponentSystem::Creator);
#ifdef BUILTINDEFAULTCOMPONENTLIB
    register_components(mpComponentFactory);
#endif

    //Check for register errors and status
    checkClassFactoryStatus(mpComponentFactory);
    checkClassFactoryStatus(mpNodeFactory);

    //Clear factory status
    mpComponentFactory->clearRegisterStatusMap();
    mpNodeFactory->clearRegisterStatusMap();


    //Do some other stuff
    mpMessageHandler->addInfoMessage("HopsanCore, Version: " + string(HOPSANCOREVERSION));
    hopsanLogFile.open("hopsan_logfile.txt");
    hopsanLogFile << "This file logs the actions done by HopsanCore,\nto trace a program crash one can see what was the last logged action.\nLook at the last rows in this file.\n\n\n";
}

//! @brief HopsanEssentials Constructor
HopsanEssentials::HopsanEssentials()
{
    mpNodeFactory = new NodeFactory;
    mpComponentFactory = new ComponentFactory;
    mpMessageHandler = getCoreMessageHandlerPtr();
    mExternalLoader.setFactory(mpComponentFactory, mpNodeFactory);
    initialize();
}

//! @brief Get a pointer to the HopsanEssentials Singelton, create it if it does not already exist.
HopsanEssentials* HopsanEssentials::getInstance()
{
    if(! mHasInstance)
    {
        mpInstance = new HopsanEssentials();
        mHasInstance = true;
        return mpInstance;
    }
    else
    {
        return mpInstance;
    }
}

//! @brief HopsanEssentials Destructor
HopsanEssentials::~HopsanEssentials()
{
    //Clear the factories
    //! @todo need to make sure that every one has destoyed all components/nodes before we unregister them, it probably cant be done from inside here
    std::cout << "Clearing factories" << std::endl;
    mpNodeFactory->clearFactory();
    mpComponentFactory->clearFactory();
    hopsanLogFile.close();

    delete mpNodeFactory;
    delete mpComponentFactory;

    mHasInstance = false;
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
        pComp->setTypeName(rTypeName);
        pComp->setName(rTypeName);
    }
    else
    {
        checkClassFactoryStatus(mpComponentFactory);
        mpComponentFactory->clearRegisterStatusMap();
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


//! @brief Creates a ComponentSystem
//! @returns A pointer to the ComponentSystem created
ComponentSystem* HopsanEssentials::createComponentSystem()
{
    return static_cast<ComponentSystem*>(createComponent("Subsystem"));
    //return new ComponentSystem();
}

//! @brief Creates a Node of given node type
//! @param [in] rNodeType The type of node to create
//! @returns A pointer to the created node
Node* HopsanEssentials::createNode(const NodeTypeT &rNodeType)
{
    Node *pNode = mpNodeFactory->createInstance(rNodeType.c_str());
    if (pNode)
    {
        pNode->mNodeType = rNodeType;
    }
    else
    {
        checkClassFactoryStatus(mpNodeFactory);
        mpNodeFactory->clearRegisterStatusMap();
    }
    return pNode;
}

//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [in,out] rStartTime A reference to the starttime variable
//! @param [in,out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* HopsanEssentials::loadHMFModel(const string filePath, double &rStartTime, double &rStopTime)
{
    return loadHopsanModelFile(filePath, this, rStartTime, rStopTime);
}


SimulationHandler *HopsanEssentials::getSimulationHandler()
{
    return &mSimulationHandler;
}

//! @brief Get the message waiting on the message queue
//! @param [out] rMessage A reference to the message string
//! @param [out] rType A reference to the message type string
//! @param [out] rTag A reference to the message type Tag
void HopsanEssentials::getMessage(std::string &rMessage, std::string &rType, std::string &rTag)
{
    HopsanCoreMessage msg = mpMessageHandler->getMessage();
    rMessage = msg.mMessage;
    rTag = msg.mTag;

    switch (msg.mType)
    {
    case HopsanCoreMessage::Error:
        rType = "error";
        break;
    case HopsanCoreMessage::Warning:
        rType = "warning";
        break;
    case HopsanCoreMessage::Info:
        rType = "info";
        break;
    case HopsanCoreMessage::Debug:
        rType = "debug";
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
    return mExternalLoader.load(path);
}

//! @brief Unloads an external component library
//! @param [in] path The path to the library DLL or SO file to unload
//! @returns True if unloaded sucessfully, otherwise false
bool HopsanEssentials::unLoadExternalComponentLib(const std::string path)
{
    return mExternalLoader.unLoad(path);
}

//! @brief Get the libNames of the currently loaded libs (the names compiled into libs)
//! @param [out] rLibNames A reference to the vector that will contain the lib names
void HopsanEssentials::getExternalComponentLibNames(std::vector<std::string> &rLibNames)
{
    mExternalLoader.getLoadedLibNames(rLibNames);
}

//! @brief Adds a message to the HopsanCore runtime log
void hopsan::addLogMess(const std::string log)
{
    hopsanLogFile << log << "\n";
}
