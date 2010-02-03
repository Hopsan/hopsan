//!
//! @file   HopsanCore.h
//! @author <peter.nordin@liu.se>
//! @date   2010-01-06
//!
//! @brief Main HopsanCore include file
//!
//! Includes all necessary HopsanCore header files and contains the HopsanEssential class.
//! One object of the class should be used in a main program to access HopsanCore component cretion functions, external lib loader and HopsanCore initialization.
//!
//$Id$

#ifndef HOPSANCORE_H_INCLUDED
#define HOPSANCORE_H_INCLUDED

#include "CoreUtilities/LoadExternal.h"
#include "Port.h"
#include "Nodes/Nodes.h"
#include "Components/Components.h"
#include <string>

//! @class HopsanEssentials
//! @brief This class gives access to HopsanCore Initialization, externalLib loading and ComponentCreation.
class HopsanEssentials
{
private:
    bool isInitialized; ///TODO: not used right now
    NodeFactory* mpNodeFactory;
    ComponentFactory* mpComponentFactory;

    void Initialize()
    {
        //Make sure that internal Nodes and Components register
        register_nodes(mpNodeFactory);
        register_components(mpComponentFactory);


        //Do some other stuff
    }

public:
    ///TODO: make sure this object can only be instansiated once either by singleton pattern or by assert flase if isInitialized
    LoadExternal externalLoader;

    HopsanEssentials()
    {
        mpNodeFactory = getCoreNodeFactoryPtr();
        mpComponentFactory = getCoreComponentFactoryPtr();
        externalLoader.setFactory(mpComponentFactory, mpNodeFactory);
        Initialize();
    }

    ~HopsanEssentials()
    {
        //Clear the factories
        ///TODO: need to make sure that every one has destoyed all components/nodes before we unregister them, it probably cant be done from inside here
        std::cout << "Clearing factories" << std::endl;
        mpNodeFactory->ClearFactory();
        mpComponentFactory->ClearFactory();
    }

    //!Creates a component with the specified key-value and returns a pointer to this component.
    Component* CreateComponent(const string &rString)
    {
            return mpComponentFactory->CreateInstance(rString.c_str());
    }

    //!TODO: for now a ugly special fix for component system, (It can not be created by the factory that only deals with Component* objects)
    ComponentSystem* CreateComponentSystem()
    {
        return new ComponentSystem();
    }
};

#endif // HOPSANCORE_H_INCLUDED
