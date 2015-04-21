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

//!
//! @file   defaultComponentLibraryInternal.cc
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
