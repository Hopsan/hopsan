//!
//! @file   HopsanEssentials.cc
//! @author <peter.nordin@liu.se>
//! @date   2010-02-19
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

//!
//! \mainpage
//! Library used in HOPSAN CORE is 'tbb 2.2'
//!

#include "HopsanEssentials.h"
#include "Components/Components.h"
#include "Nodes/Nodes.h"
#include "version.h"

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


    //Do some other stuff
    mpMessageHandler->addInfoMessage("HopsanCore, Version: " + string(HOPSANCOREVERSION));
}


HopsanEssentials::HopsanEssentials()
{
    mpNodeFactory = getCoreNodeFactoryPtr();
    mpComponentFactory = getCoreComponentFactoryPtr();
    mpMessageHandler = getCoreMessageHandlerPtr();
    externalLoader.setFactory(mpComponentFactory, mpNodeFactory);
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
    mpNodeFactory->ClearFactory();
    mpComponentFactory->ClearFactory();

    mHasInstance = false;
}


//!Creates a component with the specified key-value and returns a pointer to this component.
Component* HopsanEssentials::CreateComponent(const string &rString)
{
        return mpComponentFactory->CreateInstance(rString.c_str());
}


//! @todo for now a ugly special fix for component system, (It can not be created by the factory that only deals with Component* objects)
ComponentSystem* HopsanEssentials::CreateComponentSystem()
{
    return new ComponentSystem();
}


HopsanCoreMessage HopsanEssentials::getMessage()
{
    return mpMessageHandler->getMessage();
}


size_t HopsanEssentials::checkMessage()
{
    return mpMessageHandler->nWaitingMessages();
}
