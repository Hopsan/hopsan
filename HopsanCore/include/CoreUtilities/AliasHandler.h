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
//! @file   AliasHandler.h
//! @author Peter Nordin
//!
//! @brief Contains the alias handler help class
//!
//$Id$

#ifndef ALIASHANDLER_H
#define ALIASHANDLER_H

#include <vector>
#include <map>

#include "win32dll.h"
#include "HopsanTypes.h"

namespace hopsan {

class ComponentSystem;

class HOPSANCORE_DLLAPI AliasHandler
{
public:
    AliasHandler(ComponentSystem *pSystem);
    HString getVariableAlias(const HString &rCompName, const HString &rPortName, const HString &rVarName);
    bool setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const HString &rVarName);
    bool setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const int varId);
    bool setParameterAlias(const HString &rAlias, const HString &rCompName, const HString &rParameterName);
    void componentRenamed(const HString &rOldCompName, const HString &rNewCompName);
    void portRenamed(const HString &rCompName, const HString &rOldPortName, const HString &rNewPortName);
    void componentRemoved(const HString &rCompName);
    void portRemoved(const HString &rCompName, const HString &rPortName);
    bool hasAlias(const HString &rAlias);
    bool removeAlias(const HString &rAlias);

    std::vector<HString> getAliases() const;

    void getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, int &rVarId);
    void getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, HString &rVarName);
    void getParameterFromAlias(const HString &rAlias, HString &rCompName, HString &rParameterName);

private:
    enum {Parameter, Variable};
    typedef struct _ParamOrVariable
    {
        int type;
        HString componentName;
        HString name;
    } ParamOrVariableT;

    typedef std::map<HString, ParamOrVariableT> AliasMapT;
    AliasMapT mAliasMap;
    ComponentSystem *mpSystem;
};

}

#endif // ALIASHANDLER_H
