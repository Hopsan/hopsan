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
//! @file   LoadExternal.h
//! @author <peter.nordin@liu.se>
//! @date   2009-12-22
//!
//! @brief Contains the ExternalLoader class
//!
//$Id$

#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include "win32dll.h"
#include "Component.h"

namespace hopsan {

//Forward Declaration
class HopsanCoreMessageHandler;

class LoadedLibInfo
{
public:
    void* mpLib;
    HString mLibName;
    std::vector<HString> mRegistredComponents;
    std::vector<HString> mRegistredNodes;
};

//! @brief This class handles loading and unloading of external component and node libs
class LoadExternal
{
private:
    ComponentFactory *mpComponentFactory;
    NodeFactory *mpNodeFactory;
    HopsanCoreMessageHandler *mpMessageHandler;

    typedef std::map<HString, LoadedLibInfo> LoadedExtLibsMapT;
    LoadedExtLibsMapT mLoadedExtLibsMap;

public:
    LoadExternal(ComponentFactory* pComponentFactory, NodeFactory* pNodefactory, HopsanCoreMessageHandler *pMessenger);
    bool load(const HString &rLibpath);
    bool unLoad(const HString &rLibpath);
    void setFactory();
    void getLoadedLibNames(std::vector<HString> &rLibNames);
    void getLibContents(const HString &rLibpath, std::vector<HString> &rComponents, std::vector<HString> &rNodes);
    void getLibPathByTypeName(const HString &rTypeName, HString &rLibPath);
};
}

#endif // LOADEXTERNAL_H
