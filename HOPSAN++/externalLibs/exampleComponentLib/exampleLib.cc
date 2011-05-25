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

// Include your component code code files here
// If you have a lot of them you can include them in a separate .h file
// and then include that file here instead.

#include "component_code/myWickedOrifice.hpp"
#include "component_code/myWickedVolume.hpp"

// You need to include ComponentEssentials.h in order to gain access to the register function and the Factory types
// Also use the hopsan namespace

#include "ComponentEssentials.h"
using namespace hopsan;

extern "C" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)
{
    //Register Components
    cfact_ptr->registerCreatorFunction("MyWickedOrifice", MyWickedOrifice::Creator);
    cfact_ptr->registerCreatorFunction("MyWickedVolume", MyWickedVolume::Creator);

    //Register custom nodes (if any)
}
