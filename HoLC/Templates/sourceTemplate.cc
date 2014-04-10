// Include headers from HopsanCore
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

// Include component code files
<<<includecomponents>>>

using namespace hopsan;

//Register components
extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    // Register Components
    <<<registercomponents>>>

    // Register Custom Nodes (not yet supported)
    HOPSAN_UNUSED(pNodeFactory);
}

//Provide library information for Hopsan
extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->libName = (char*)"<<<libname>>>";

    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;
}
