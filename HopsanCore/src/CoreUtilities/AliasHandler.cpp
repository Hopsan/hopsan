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
//! @file   AliasHandler.cpp
//! @author Peter Nordin
//!
//! @brief Contains the alias handler help class
//!
//$Id$

#include <vector>
#include <map>

#include "win32dll.h"
#include "HopsanTypes.h"

#include "CoreUtilities/AliasHandler.h"
#include "ComponentSystem.h"
#include "CoreUtilities/StringUtilities.h"

#include <vector>

using namespace hopsan;
using namespace std;

AliasHandler::AliasHandler(ComponentSystem *pSystem)
{
    mpSystem = pSystem;
}

HString AliasHandler::getVariableAlias(const HString &rCompName, const HString &rPortName, const HString &rVarName)
{
    Component *pComp = mpSystem->getSubComponent(rCompName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(rPortName);
        if (pPort)
        {
            int id = pPort->getNodeDataIdFromName(rVarName);
            return pPort->getVariableAlias(id);
        }
    }
    return "";
}

//! @todo maybe this should be the default version, right now search comp/port twice
bool AliasHandler::setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const HString &rVarName)
{
    Component *pComp = mpSystem->getSubComponent(rCompName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(rPortName);
        if (pPort)
        {
            if (pPort->isMultiPort())
            {
                if ( !rAlias.empty() )
                {
                    mpSystem->addErrorMessage("You can not set alias on multiport variables");
                }
                return false;
            }

            int id = pPort->getNodeDataIdFromName(rVarName);
            return setVariableAlias(rAlias, rCompName, rPortName, id);
        }
    }
    return false;
}

bool AliasHandler::setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const int varId)
{
    if (varId<0)
    {
        mpSystem->addErrorMessage("Can not set alias for dataId < 0 (incorrect variable name)");
        return false;
    }

    if (!isNameValid(rAlias))
    {
        mpSystem->addErrorMessage("Invalid characters in requested alias name: "+rAlias);
        return false;
    }

    //! @todo must check if existing alias is set for the same component that already have it to avoid warning
    // Check if alias already exist
    if (hasAlias(rAlias))
    {
        HString comp,port;
        int var;
        getVariableFromAlias(rAlias,comp,port,var);
        if ( (comp==rCompName) && (port==rPortName) && (var==varId) )
        {
            // We are setting the same alias again, skip without warning
            return true;
        }
        else
        {
            // The alias already exist somewhere else
            mpSystem->addErrorMessage("Alias: "+rAlias+" already exist");
            return false;
        }
    }

    if (mpSystem->hasReservedUniqueName(rAlias))
    {
        mpSystem->addErrorMessage("The alias: " + rAlias + " is already used as some other name");
        return false;
    }

    // Set the alias for the given component port and var
    Component *pComp = mpSystem->getSubComponent(rCompName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(rPortName);
        if (pPort)
        {
            // First unregister the old alias (if it exists)
            HString prevAlias = pPort->getVariableAlias(varId);
            if (!prevAlias.empty())
            {
                //! @todo the remove will search for port again all the way, maybe have a special remove to use when we know the port and id already
                removeAlias(prevAlias);
            }

            // If alias is non empty, set it
            if (!rAlias.empty())
            {
                //! @todo do we need to check if this is OK ??
                pPort->setVariableAlias(rAlias, varId);

                ParamOrVariableT data = {Variable, rCompName, rPortName};
                mAliasMap.insert(std::pair<HString, ParamOrVariableT>(rAlias, data));
                mpSystem->reserveUniqueName(rAlias);
            }
            return true;
        }
    }
    mpSystem->addErrorMessage("Component or Port not found when setting alias");
    return false;
}

bool AliasHandler::setParameterAlias(const HString & /*alias*/, const HString & /*compName*/, const HString & /*parameterName*/)
{
    mpSystem->addErrorMessage("AliasHandler::setParameterAlias has not been implemented");
    return false;
}

void AliasHandler::componentRenamed(const HString &rOldCompName, const HString &rNewCompName)
{
    std::map<HString, ParamOrVariableT>::iterator it=mAliasMap.begin();
    while(it!=mAliasMap.end())
    {
        if (it->second.componentName == rOldCompName)
        {
            HString alias = it->first;
            ParamOrVariableT data = it->second;
            mAliasMap.erase(it);
            data.componentName = rNewCompName;

            // Re-insert data (with new comp name)
            mAliasMap.insert(std::pair<HString, ParamOrVariableT>(alias, data));

            // Restart search for more components
            it = mAliasMap.begin();
        }
        else
        {
            ++it;
        }
    }
}

void AliasHandler::portRenamed(const HString & /*compName*/, const HString & /*oldPortName*/, const HString & /*newPortName*/)
{
    mpSystem->addErrorMessage("AliasHandler::portRenamed has not been implemented");
}

void AliasHandler::componentRemoved(const HString &rCompName)
{
    std::map<HString, ParamOrVariableT>::iterator it=mAliasMap.begin();
    while (it!=mAliasMap.end())
    {
        if (it->second.componentName == rCompName)
        {
            removeAlias(it->first);
            it = mAliasMap.begin(); //Restart search for more components
        }
        else
        {
            ++it;
        }
    }
}

void AliasHandler::portRemoved(const HString & /*compName*/, const HString & /*portName*/)
{
    mpSystem->addErrorMessage("AliasHandler::portRemoved has not been implemented");
}

bool AliasHandler::hasAlias(const HString &rAlias)
{
    if (mAliasMap.count(rAlias)>0)
    {
        return true;
    }
    return false;
}

bool AliasHandler::removeAlias(const HString &rAlias)
{
    std::map<HString, ParamOrVariableT>::iterator it = mAliasMap.find(rAlias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            Component *pComp = mpSystem->getSubComponent(it->second.componentName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(it->second.name);
                if (pPort)
                {
                    int id = pPort->getVariableIdByAlias(rAlias);
                    pPort->setVariableAlias("",id); //Remove variable alias
                }
            }
        }
        mpSystem->unReserveUniqueName(rAlias); //We must unreserve before erasing the it, since rAlias may be a reference to data in it
        mAliasMap.erase(it);
        return true;
    }
    return false;
}

std::vector<HString> AliasHandler::getAliases() const
{
    vector<HString> aliasNames;
    aliasNames.reserve(mAliasMap.size());

    std::map<HString, ParamOrVariableT>::const_iterator it;
    for (it=mAliasMap.begin(); it!=mAliasMap.end(); ++it)
    {
        aliasNames.push_back(it->first);
    }
    return aliasNames;
}

void AliasHandler::getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, int &rVarId)
{
    // Clear return vars to indicate any failure
    rCompName.clear(); rPortName.clear(); rVarId=-1;

    // Search through map for specified alias
    std::map<HString, ParamOrVariableT>::iterator it;
    it = mAliasMap.find(rAlias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            rCompName = it->second.componentName;
            rPortName = it->second.name;

            // Lookup varName from port
            Component* pComp = mpSystem->getSubComponent(rCompName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(rPortName);
                if (pPort)
                {
                    rVarId = pPort->getVariableIdByAlias(rAlias);
                }
            }
        }
    }
}

void AliasHandler::getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, HString &rVarName)
{
    // Clear return vars to indicate any failure
    rCompName.clear(); rPortName.clear(); rVarName.clear();

    // Search through map for specified alias
    AliasMapT::iterator it = mAliasMap.find(rAlias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            rCompName = it->second.componentName;
            rPortName = it->second.name;

            // Lookup varName from port
            Component* pComp = mpSystem->getSubComponent(rCompName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(rPortName);
                if (pPort)
                {
                    int id = pPort->getVariableIdByAlias(rAlias);
                    const NodeDataDescription *pDataDesc = pPort->getNodeDataDescription(id);
                    if (pDataDesc)
                    {
                        rVarName = pDataDesc->name;
                    }
                }
            }
        }
    }
}

void AliasHandler::getParameterFromAlias(const HString & /*alias*/, HString &/*rCompName*/, HString &/*rParameterName*/)
{
    mpSystem->addErrorMessage("AliasHandler::getParameterFromAlias has not been implemented");
}

