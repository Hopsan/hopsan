#ifndef HOPSANCORE_H_INCLUDED
#define HOPSANCORE_H_INCLUDED

#include "CoreUtilities/LoadExternal.h"
#include "Nodes/Nodes.h"
#include "Components/Components.h"
#include <string>


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

    Component* CreateComponent(const string &rString)
    {
        return mpComponentFactory->CreateInstance(rString.c_str());
    }
};

#endif // HOPSANCORE_H_INCLUDED
