#ifndef HOPSANCORE_H_INCLUDED
#define HOPSANCORE_H_INCLUDED

#include "Component.h"
#include "Node.h"
#include "Nodes/Nodes.h"
#include "CoreUtilities/LoadExternal.h"

#include "Components/PressureSource.hpp"
#include "Components/Orifice.hpp"
#include "Components/Volume.hpp"
#include "Components/TLMlossless.hpp"
#include "Components/TLMRlineR.hpp"
#include "Components/PressureSourceQ.hpp"
#include "Components/FlowSourceQ.hpp"
#include "Components/Source.hpp"
#include "Components/Sink.hpp"
#include "Components/Gain.hpp"
#include "Components/TurbOrifice.hpp"

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

    ComponentFactory* getComponentFactoryPtr()
    {
        return mpComponentFactory;
    }
};

#endif // HOPSANCORE_H_INCLUDED
