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
