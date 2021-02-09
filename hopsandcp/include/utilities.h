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
//! @file   utilities.h
//! @brief Contains utilities for hopsandcp
//!
//$Id$

#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include "HopsanEssentials.h"



enum SaveResults {Final, Full};
void generateFullSubSystemHierarchyName(const hopsan::ComponentSystem *pSys, hopsan::HString &rFullSysName, const hopsan::HString &separator);
hopsan::HString generateFullSubSystemHierarchyName(const hopsan::Component *pComponent, const hopsan::HString &separator, bool includeLastSeparator=true);
void saveResultsToCSV(hopsan::ComponentSystem *pRootSystem, const std::string &rFileName, const SaveResults howMany, const std::vector<std::string>& includeFilter);

template <typename ContainerT, typename ValueT>
bool contains(const ContainerT& container, const ValueT& value) {
    auto it = std::find(container.begin(), container.end(), value);
    return (it != container.end());
}

template <typename SaveTimeFunc, typename SaveVariableFunc>
void saveResultsTo(hopsan::ComponentSystem *pCurrentSystem, const std::vector<std::string>& includeFilter, SaveTimeFunc writeTime,
                   SaveVariableFunc writeVariable )
{
    hopsan::HString prefix = generateFullSubSystemHierarchyName(pCurrentSystem, "$"); //!< @todo not necessary every time if we use recursion instead

    // Use names to get components in alphabeteical order
    const std::vector<hopsan::HString> componentNames = pCurrentSystem->getSubComponentNames();
    size_t numberOfLoggedComponentsInThisSystem = 0;
    for (const auto& name : componentNames) {
        hopsan::Component *pComponent = pCurrentSystem->getSubComponent(name);

        size_t numberOfLoggedVariablesInThisComponent = 0;
        const std::vector<hopsan::Port*> ports = pComponent->getPortPtrVector();
        for (const hopsan::Port* pPort : ports) {
            // Ignore ports that have logging disabled
            if (!pPort->isLoggingEnabled()) {
                continue;
            }

            hopsan::HString fullPortName = prefix + pComponent->getName() + "#" + pPort->getName();
            const bool includeAllVariablesInThisPort = (includeFilter.empty() || contains(includeFilter, fullPortName.c_str())) &&
                    pPort->isLoggingEnabled();

            const std::vector<hopsan::NodeDataDescription> *pVariables = pPort->getNodeDataDescriptions(); // TODO do not return pointer
            if (pVariables)
            {
                for (size_t v=0; v<pVariables->size(); ++v)
                {
                    const hopsan::NodeDataDescription* pVariable = &pVariables->at(v);

                    // Create data vector
                    const std::vector< std::vector<double> > *pLogData = pPort->getLogDataVectorPtr();
                    if(pLogData == nullptr || pLogData->empty()) {
                        continue;
                    }

                    hopsan::HString fullVarName = fullPortName+"#"+pVariable->name;
                    const bool includeThisVariable = includeAllVariablesInThisPort || contains(includeFilter, fullVarName.c_str());
                    if (!includeThisVariable) {
                        continue;
                    }

                    // Add variable to exporter
                    writeVariable(pCurrentSystem, pComponent, pPort, v);

                    numberOfLoggedVariablesInThisComponent++;
                }
            }
        }
        if (numberOfLoggedVariablesInThisComponent > 0) {
            numberOfLoggedComponentsInThisSystem++;
        }

        // Recurse into subsystems
        if (pComponent->isComponentSystem()) {
            saveResultsTo(dynamic_cast<hopsan::ComponentSystem*>(pComponent), includeFilter, writeTime, writeVariable);
        }
    }

    // Save this sytems time vector
    if (numberOfLoggedComponentsInThisSystem > 0) {
        writeTime(pCurrentSystem);
    }

}
#endif // UTILITIES_H
