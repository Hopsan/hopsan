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
//! @file   defaultComponentLibrary.cpp
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_contents function that registers all built in components
//!
//$Id$

#include "defaultComponentLibraryInternal.h"
using namespace hopsan;

extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    // ========== Register Components ==========
    // Use the registerCreatorFunction(KeyValue, Function) in the component factory to register components
    // TheKeyValue is a text string with the TypeName of the component.
    // This value must be unique for every component in Hopsan.
    // If a typename is already in use, your component will not be added.
    // Suggestion, let the KeyValue (TypeName) be the same as your Class name

    hopsan::register_default_components(pComponentFactory);

    // ========== Register Custom Nodes (if any) ==========
    // These are built into Hopsan Core
    HOPSAN_UNUSED(pNodeFactory)

}

// When you load your model into Hopsan, the get_hopsan_info() function bellow will be called
// This information is used to make sure that your component and the hopsan core have the same version
extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->libName = (char*)"HopsanDefaultComponentLibrary";
    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;
}
