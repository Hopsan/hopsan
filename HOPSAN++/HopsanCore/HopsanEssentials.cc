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
#include "Components/Components.h"
#include "Nodes/Nodes.h"
#include "version.h"
#include "CoreUtilities/ClassFactoryStatusCheck.hpp"
#include <string>

using namespace std;
using namespace hopsan;

//Set the stacic start values
bool HopsanEssentials::mHasInstance = false;
HopsanEssentials* HopsanEssentials::mpInstance = 0;


void HopsanEssentials::Initialize()
{
    //Make sure that internal Nodes and Components register
    register_nodes(mpNodeFactory);
    register_components(mpComponentFactory);

    //Check for register errors and status
    checkClassFactoryStatus<ComponentFactory>(mpComponentFactory);
    checkClassFactoryStatus<NodeFactory>(mpNodeFactory);

    //Clear factory status
    mpComponentFactory->clearRegisterStatusMap();
    mpNodeFactory->clearRegisterStatusMap();


    //Do some other stuff
    mpMessageHandler->addInfoMessage("HopsanCore, Version: " + string(HOPSANCOREVERSION));
    hopsanLogFile.open("hopsan_logfile.txt");
    hopsanLogFile << "This file logs the actions done by HopsanCore,\nto trace a program crash one can see what was the last logged action.\nLook at the last rows in this file.\n\n\n";
}


HopsanEssentials::HopsanEssentials()
{
    mpNodeFactory = new NodeFactory; //getCoreNodeFactoryPtr();
    mpComponentFactory = new ComponentFactory;//getCoreComponentFactoryPtr();
    mpMessageHandler = getCoreMessageHandlerPtr();
    mExternalLoader.setFactory(mpComponentFactory, mpNodeFactory);
    Initialize();
}


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

//! Returns the hopsa core version as a string
std::string HopsanEssentials::getCoreVersion()
{
    return HOPSANCOREVERSION;
}

//! Creates a component with the specified key-value and returns a pointer to this component.
Component* HopsanEssentials::CreateComponent(const string &rString)
{
    addLogMess(rString + "::CreateComponent");
    Component* pComp = mpComponentFactory->createInstance(rString.c_str());
    if (pComp)
    {
        pComp->setTypeName(rString);
    }
    else
    {
        checkClassFactoryStatus<ComponentFactory>(mpComponentFactory);
        mpComponentFactory->clearRegisterStatusMap();
    }
    return pComp;
}

bool HopsanEssentials::hasComponent(const string type)
{
    return mpComponentFactory->hasKey(type.c_str());
}


//! @todo for now a ugly special fix for component system, (It can not be created by the factory that only deals with Component* objects)
ComponentSystem* HopsanEssentials::CreateComponentSystem()
{
    return new ComponentSystem();
}

Node* HopsanEssentials::createNode(const NodeTypeT &rNodeType)
{
    Node *pNode = mpNodeFactory->createInstance(rNodeType.c_str());
    if (pNode)
    {
        pNode->mNodeType = rNodeType;
    }
    else
    {
        checkClassFactoryStatus<NodeFactory>(mpNodeFactory);
        mpNodeFactory->clearRegisterStatusMap();
    }
    return pNode;
}


HopsanCoreMessage HopsanEssentials::getMessage()
{
    return mpMessageHandler->getMessage();
}


size_t HopsanEssentials::checkMessage()
{
    return mpMessageHandler->nWaitingMessages();
}

bool HopsanEssentials::loadExternalComponent(const string path)
{
    return mExternalLoader.load(path);
}

void hopsan::addLogMess(std::string log)
{
    hopsanLogFile << log << "\n";
}
