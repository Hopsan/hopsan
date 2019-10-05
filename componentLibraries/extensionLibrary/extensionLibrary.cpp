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

//!
//! @file   extensionLibrary.cpp
//!
//$Id$

// Include automatically generated header code for all default library components
#include "Component.h"
#include "HopsanCoreVersion.h"
#include "Components.h"

extern "C" DLLEXPORT void register_contents(hopsan::ComponentFactory* pComponentFactory, hopsan::NodeFactory* pNodeFactory)
{
    // Include automatically generated registration code for all default library components
    #include "Components.cci"

    // ========== Register Custom Nodes (if any) ==========
    HOPSAN_UNUSED(pNodeFactory)
}

// When you load your model into Hopsan, the get_hopsan_info() function bellow will be called
// This information is used to make sure that your component and the Hopsan core have the same version
extern "C" DLLEXPORT void get_hopsan_info(hopsan::HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->libName = (char*)"HopsanExtensionLibrary";
    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;
}
