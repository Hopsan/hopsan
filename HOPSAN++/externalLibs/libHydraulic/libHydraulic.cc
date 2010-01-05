#include "libHydraulic.h"
#include "Component.h"
#include "win32dll.h"

extern "C" DLLEXPORT void register_contents(ComponentFactory::FactoryVectorT *factory_vector_ptr)
{
    std::cout << "Running register function in dll" << std::endl;
    factory_vector_ptr->push_back(ComponentFactory::FactoryPairT("ComponentExternalOrifice", ComponentExternalOrifice::Creator));
    factory_vector_ptr->push_back(ComponentFactory::FactoryPairT("ComponentExternalVolume", ComponentExternalVolume::Creator));
}
