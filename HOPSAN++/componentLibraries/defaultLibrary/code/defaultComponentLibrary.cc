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
//! @file   defaultComponentLibrary.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_components function that registers all built in components
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

    hopsan::register_components(pComponentFactory);

    // ========== Register Custom Nodes (if any) ==========
    // These are built into Hopsan Core

}

// When you load your model into Hopsan, the get_hopsan_info() function bellow will be called
// This information is used to make sure that your component and the hopsan core have the same version
extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->libName = (char*)"HopsanDefaultComponentLibrary";
    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;
}
