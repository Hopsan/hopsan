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

//Forward Declaration
class HopsanCoreMessageHandler;

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
    HopsanCoreMessageHandler *mpMessageHandler;

    typedef std::map<std::string, LoadedLibInfo> LoadedExtLibsMapT;
    LoadedExtLibsMapT mLoadedExtLibsMap;

public:
    LoadExternal(ComponentFactory* pComponentFactory, NodeFactory* pNodefactory, HopsanCoreMessageHandler *pMessenger);
    bool load(const std::string &rLibpath);
    bool unLoad(const std::string &rLibpath);
    void setFactory();
    void getLoadedLibNames(std::vector<std::string> &rLibNames);
    void getLibContents(const std::string libpath, std::vector<std::string> &rComponents, std::vector<std::string> &rNodes);
};
}

#endif // LOADEXTERNAL_H
