#include "libSignal.h"
#include "Component.h"
#include "win32dll.h"

extern "C" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)
{
    std::cout << "Running register function in dll" << std::endl;
    //Register Components
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSource", ComponentExternalSource::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalGain", ComponentExternalGain::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSink", ComponentExternalSink::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalStep", ComponentExternalStep::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSineWave", ComponentExternalSineWave::Creator);

    //Register custom nodes (if any)
}
