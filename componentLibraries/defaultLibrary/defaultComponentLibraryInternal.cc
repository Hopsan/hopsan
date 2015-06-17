/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
