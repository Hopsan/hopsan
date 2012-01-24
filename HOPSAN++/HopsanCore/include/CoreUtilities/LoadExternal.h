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
//! @file   LoadExternal.h
//! @author <peter.nordin@liu.se>
//! @date   2009-12-22
//!
//! @brief Contains the ExternalLoader class
//!
//$Id$

#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include <string>
#include "win32dll.h"
#include "Component.h"

namespace hopsan {

class LoadedLibInfo
{
public:
    void* mpLib;
    std::string mLibName;
    std::vector<std::string> mRegistredComponents;
    std::vector<std::string> mRegistredNodes;
};

//! @brief This class handles loading and unloading of external component and node libs
class LoadExternal
{
private:
    ComponentFactory *mpComponentFactory;
    NodeFactory *mpNodeFactory;

    typedef std::map<std::string, LoadedLibInfo> LoadedExtLibsMapT;
    LoadedExtLibsMapT mLoadedExtLibsMap;

public:
    bool load(const std::string libpath);
    bool unLoad(const std::string libpath);
    void setFactory(ComponentFactory* cfactory_ptr, NodeFactory* nfactory_ptr);
    void getLoadedLibNames(std::vector<std::string> &rLibNames);
};
}

#endif // LOADEXTERNAL_H
