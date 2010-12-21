#include "atlascopco.h"
#include "../../HopsanCore/ComponentEssentials.h"
using namespace hopsan;

extern "C" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)
{
    std::cout << "Running register function in atlascopco dll" << std::endl;

    //Register Components
    cfact_ptr->registerCreatorFunction("bar", bar::Creator);
    cfact_ptr->registerCreatorFunction("con2", con2::Creator);
    cfact_ptr->registerCreatorFunction("fsrc", fsrc::Creator);
    cfact_ptr->registerCreatorFunction("sep2", sep2::Creator);
    cfact_ptr->registerCreatorFunction("vsrc", vsrc::Creator);

    //Register custom nodes (if any)
}
