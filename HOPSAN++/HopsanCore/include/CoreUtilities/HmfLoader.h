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
//! @file   HmfLoader.cc
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-03-20
//!
//! @brief Contains the HopsanCore hmf loader functions
//!
//$Id$

#ifndef HMFLOADER_H_INCLUDED
#define HMFLOADER_H_INCLUDED


#include <string>

#include "win32dll.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
//#include "rapidxml_print.hpp"



namespace hopsan {
    //Forward declaration
    class ComponentSystem;

    class DLLIMPORTEXPORT HmfLoader
    {
    public:
        HmfLoader();
        ComponentSystem* loadModel(std::string filename, double &rStartTime, double &rStopTime);

    private:
        void loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem *pSystem);
        void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem *pSystem);
        void loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem *pSystem);

    };
}

#endif

