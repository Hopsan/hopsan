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
//! @file   defaultComponentLibrary.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_components function that registers all built in components
//!
//$Id$

#ifndef DEFAULTCOMPONENTLIBRARY_H
#define DEFAULTCOMPONENTLIBRARY_H

#include "ComponentEssentials.h"

namespace hopsan {

    DLLIMPORTEXPORT void register_components(ComponentFactory* pComponentFactory);
}

#endif // DEFAULTCOMPONENTLIBRARY_H
