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
//! @file   defaultComponents.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Includes all built in components
//!
//$Id$

#ifndef DEFAULTCOMPONENTS_H_INCLUDED
#define DEFAULTCOMPONENTS_H_INCLUDED

/* Special Components */
#include "Special/HopsanDefaultSpecialComponents.h"

/* Connectivity Components */
#include "Connectivity/HopsanDefaultconnectivityComponents.h"

/* Hydraulic Components */
#include "Hydraulic/HopsanDefaultHydraulicComponents.h"

/* Electric Components */
#include "Pneumatic/HopsanDefaultPneumaticComponents.h"

/* Signal Components */
#include "Signal/HopsanDefaultSignalComponents.h"

/* Mechanical Components */
#include "Mechanic/HopsanDefaultMechanicComponents.h"

/* Electric Components */
#include "Electric/HopsanDefaultElectricComponents.h"

#endif // DEFAULTCOMPONENTS_H_INCLUDED
