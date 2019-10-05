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

// Include your component code code files here
// If you have lots of them you can include them in separate .h files and then include those files here instead.

#include "HydraulicComponents/MyExampleOrifice.hpp"
#include "HydraulicComponents/MyExampleConstantOrifice.hpp"
#include "HydraulicComponents/MyExampleVolume.hpp"
#include "HydraulicComponents/MyExampleVolume2.hpp"
#include "SignalComponents/MyExampleSignalSum.hpp"

// You need to include ComponentEssentials.h in order to gain access to the register function and the Factory types
// Also use the hopsan namespace

#include "ComponentEssentials.h"
using namespace hopsan;

// When you load your model into Hopsan, the register_contents() function bellow will be called
// It will register YOUR components into the Hopsan ComponentFactory

extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    // ========== Register Components ==========
    // Use the registerCreatorFunction(KeyValue, Function) in the component factory to register components
    // The KeyValue is a text string with the TypeName of the component.
    // This value must be unique for every component in Hopsan.
    // If a typename is already in use, your component will not be added.
    // Suggestion, let the KeyValue (TypeName) be the same as your Class name
    // If that name is already in use, use something similar

    pComponentFactory->registerCreatorFunction("MyExampleOrifice", MyExampleOrifice::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleConstantOrifice", MyExampleConstantOrifice::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleVolume", MyExampleVolume::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleVolume2", MyExampleVolume2::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleSignalSum", MyExampleSignalSum::Creator);

    // ========== Register Custom Nodes (if any) ==========
    // This is not yet supported
    HOPSAN_UNUSED(pNodeFactory)
}

// When you load your model into Hopsan, the get_hopsan_info() function bellow will be called
// This information is used to make sure that your component and the hopsan core have the same version

extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    // Change the name of the lib to something unique
    // You can include numbers in your name to indicate library version (if you want)
    pHopsanExternalLibInfo->libName = (char*)"HopsanExampleComponentLibrary";

    // Leave these two lines as they are
    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;

}
