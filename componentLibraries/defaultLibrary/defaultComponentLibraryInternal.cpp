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
//! @file   defaultComponentLibraryInternal.cpp
//! @author FluMeS
//! @date   2012-01-13
//! @brief Contains the register_default_components function that registers all built in components
//!
//$Id$

#include "defaultComponentLibraryInternal.h"

// Include automatically generated header code for all default library components
#include "Components.h"

//! @defgroup Components Components
//!
//! @defgroup HydraulicComponents HydraulicComponents
//! @ingroup Components
//!
//! @defgroup MechanicalComponents MechanicalComponents
//! @ingroup Components
//!
//! @defgroup SignalComponents SignalComponents
//! @ingroup Components
//!
//! @defgroup ElectricComponents ElectricComponents
//! @ingroup Components

using namespace hopsan;

//!
//! @brief Registers the creator function of all built in components
//! @param [in,out] pComponentFactory A pointer the the component factory in wich to register the components
//!
void hopsan::register_default_components(ComponentFactory* pComponentFactory)
{
    // Include automatically generated registration code for all default library components
    #include "Components.cci"


    // ========== Additional Components ==========

    // Here you can add your own components if you want to compile them into the default library
    // Use the following form:
    // pComponentFactory->registerCreatorFunction("TYPENAME", CLASSNAME::Creator);
    //
    // Example:
    // pComponentFactory->registerCreatorFunction("HydraulicVolume", HydraulicVolume::Creator);



}
