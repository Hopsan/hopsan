#include "libHydraulic.h"
#include "Component.h"
#include "win32dll.h"

//extern "C" DLLEXPORT void register_contents(ComponentFactory::FactoryVectorT *factory_vector_ptr)
//{
//    std::cout << "Running register function in dll" << std::endl;
//    factory_vector_ptr->push_back(ComponentFactory::FactoryPairT("ComponentExternalOrifice", ComponentExternalOrifice::Creator));
//    factory_vector_ptr->push_back(ComponentFactory::FactoryPairT("ComponentExternalVolume", ComponentExternalVolume::Creator));
//    factory_vector_ptr->push_back(ComponentFactory::FactoryPairT("ComponentExternalPressureSource", ComponentExternalPressureSource::Creator));
//    factory_vector_ptr->push_back(ComponentFactory::FactoryPairT("ComponentExternalFlowSourceQ", ComponentExternalFlowSourceQ::Creator));
//}

extern "C" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)
{
    std::cout << "Running register function in dll" << std::endl;
    //Register Components
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalOrifice", ComponentExternalOrifice::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalVolume", ComponentExternalVolume::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalPressureSource", ComponentExternalPressureSource::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalFlowSourceQ", ComponentExternalFlowSourceQ::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalPressureSourceQ", ComponentExternalPressureSourceQ::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalFixedDisplacementPump", ComponentExternalFixedDisplacementPump::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternalCheckValve", ComponentExternalCheckValve::Creator);
//    cfact_ptr->RegisterCreatorFunction("ComponentExternal43Valve", ComponentExternal43Valve::Creator);

    //Register custom nodes (if any)
}
