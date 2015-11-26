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

class DLLIMPORTEXPORT AliasHandler
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
