/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

// Include headers from HopsanCore
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

// Include component code files
#include "Hopsan2DBody2.hpp"
#include "Hopsan2DBodyTest.hpp"
#include "Hopsan2DJoint.hpp"
#include "Hopsan2DFixedAttachment.hpp"
#include "Hopsan2DForceTorqueSource.hpp"

using namespace hopsan;

//Register components
extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    // Register Components
    pComponentFactory->registerCreatorFunction("Hopsan2DBody2", Hopsan2DBody2::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DBodyTest", Hopsan2DBodyTest::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DJoint", Hopsan2DJoint::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DFixedAttachment", Hopsan2DFixedAttachment::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DForceTorqueSource", Hopsan2DForceTorqueSource::Creator);

    // Register Custom Nodes (not yet supported)
    HOPSAN_UNUSED(pNodeFactory);
}

//Provide library information for Hopsan
extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->libName = (char*)"My2DTestLib";

    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;
}
