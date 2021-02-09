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
//! @file   utilities.cpp
//! @brief Contains utilities for hopsandcp
//!

#include <iostream>
#include <fstream>
#include <algorithm>

#include "utilities.h"

#include "HopsanEssentials.h"
#include "HopsanTypes.h"

using namespace std;
using namespace hopsan;

void generateFullSubSystemHierarchyName(const ComponentSystem *pSys, HString &rFullSysName, const HString& separator)
{
    if (pSys->getSystemParent())
    {
        generateFullSubSystemHierarchyName(pSys->getSystemParent(), rFullSysName, separator);
        rFullSysName.append(pSys->getName()).append(separator);
    }
    else
    {
        // Do not include top-level name in sub-system hierarchy
        rFullSysName.clear();
    }
}

HString generateFullSubSystemHierarchyName(const Component *pComponent, const HString &separator, bool includeLastSeparator)
{
    const ComponentSystem* pSystem = pComponent->isComponentSystem() ? dynamic_cast<const ComponentSystem*>(pComponent) : pComponent->getSystemParent();

    HString fullSysName;
    generateFullSubSystemHierarchyName(pSystem, fullSysName, separator);
    if (!includeLastSeparator && !fullSysName.empty()) {
        fullSysName.erase(fullSysName.size()-separator.size(),separator.size());
    }
    return fullSysName;
}

//! @brief Save results to CSV format
//! @param [in] pRootSystem Pointer to component system
//! @param [in] rFileName File name for output file
//! @param [in] howMany Specifies if all results or only final values should be saved
//! @param [in] includeFilter list of full port names or variables names to include (excluding all others)
void saveResultsToCSV(ComponentSystem *pRootSystem, const string &rFileName, const SaveResults howMany, const std::vector<string>& includeFilter)
{
    if (pRootSystem)
    {
        ofstream outfile;
        outfile.open(rFileName.c_str());
        if (outfile.good()) {

            auto addTimeVariable = [&outfile, howMany](ComponentSystem* pSystem) {
                //! @todo alias a for time ? is that even posible
                HString parentSystemNames = generateFullSubSystemHierarchyName(pSystem,"$");
                if (howMany == Final) {
                    outfile << parentSystemNames.c_str() << "Time,,s," << std::scientific << pSystem->getTime() << endl;
                }
                else if (howMany == Full) {
                    vector<double> *pLogTimeVector = pSystem->getLogTimeVector();
                    if (pLogTimeVector->size() > 0) {
                        outfile << parentSystemNames.c_str() << "Time,,s";
                        for (size_t t=0; t<pSystem->getNumActuallyLoggedSamples(); ++t) {
                            outfile << "," << std::scientific << (*pLogTimeVector)[t];
                        }
                        outfile << endl;
                    }
                }
            };

            auto addVariable = [&outfile, howMany](const ComponentSystem* pSystem, const Component* pComponent, const Port* pPort, size_t variableIndex) {
                const NodeDataDescription& variable = *pPort->getNodeDataDescription(variableIndex);
                const vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();
                if( (pLogData != nullptr) && !pLogData->empty()) {
                    const HString fullVarName = generateFullSubSystemHierarchyName(pSystem,"$") + pComponent->getName() + "#" + pPort->getName() + "#" + variable.name;
                    if (howMany == Final) {
                        outfile << fullVarName.c_str() << "," << pPort->getVariableAlias(variableIndex).c_str() << "," << variable.unit.c_str();
                        outfile << "," << std::scientific << pPort->readNode(variableIndex) << endl;
                    }
                    else if (howMany == Full)
                    {
                        // Only write something if data has been logged (skip ports that are not logged)
                        // We assume that the data vector has been cleared
                        if (pPort->getLogDataVectorPtr()->size() > 0) {
                            outfile << fullVarName.c_str() << "," << pPort->getVariableAlias(variableIndex).c_str() << "," << variable.unit.c_str();
                            for (size_t t=0; t<pSystem->getNumActuallyLoggedSamples(); ++t) {
                                outfile << "," << std::scientific << (*pLogData)[t][variableIndex];
                            }
                            outfile << endl;
                        }
                    }
                }
            };

            saveResultsTo(pRootSystem, includeFilter, addTimeVariable, addVariable);

        }
        else {
            std::cout << "Could not open: " << rFileName << " for writing!\n";
        }

        outfile.close();
    }
}
