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
//! @file   ModelUtilities.h
//! @brief Contains model specific helpfunctions for CLI
//!
//$Id$

#ifndef MODELUTILITIES_H
#define MODELUTILITIES_H

#include <string>
#include <vector>
#include "core_cli.h"
#include "HopsanEssentials.h"

void printTsInfo(const hopsan::ComponentSystem* pSystem);
void printSystemParams(hopsan::ComponentSystem* pSystem);
void printComponentHierarchy(hopsan::ComponentSystem *pSystem, std::string prefix="",
                             const bool doPrintTsInfo=false,
                             const bool doPrintSystemParams=false);

// ===== Save Functions =====
enum SaveResults {Final, Full};
void saveResults(hopsan::ComponentSystem *pSys, const std::string &rFileName, const SaveResults howMany, const std::vector<std::string> &includeFilter,
                 std::string prefix="", std::ofstream *pFile=0);
void saveResultsToHDF5(hopsan::ComponentSystem *pSys, const std::string &rFileName, const SaveResults howMany);
void transposeCSVresults(const std::string &rFileName);
void exportParameterValuesToCSV(const std::string &rFileName, hopsan::ComponentSystem* pSystem, std::string prefix="", std::ofstream *pFile=0);

// ===== Load Functions =====
void importParameterValuesFromCSV(const std::string filePath, hopsan::ComponentSystem* pSystem);
void readNodesToSaveFromTxtFile(const std::string filePath, std::vector<std::string> &rComps, std::vector<std::string> &rPorts);

// ===== Help Functions =====
void generateFullSubSystemHierarchyName(const hopsan::ComponentSystem *pSys, hopsan::HString &rFullSysName);
hopsan::HString generateFullPortVariableName(const hopsan::Port *pPort, const size_t dataId);


hopsan::Component *getComponentWithFullName(hopsan::ComponentSystem *pRootSystem, const std::string &fullComponentName);
hopsan::Port* getPortWithFullName(hopsan::ComponentSystem *pRootSystem, const std::string &fullPortName);

template<typename PortFunction>
void forEachPort(hopsan::ComponentSystem *pRootSystem, PortFunction function )
{
    auto components = pRootSystem->getSubComponents();
    for (auto& pComponent : components)
    {
        auto ports = pComponent->getPortPtrVector();
        for (auto& pPort : ports)
        {
            function(*pPort);
        }

        // Recurse into subsystems
        if (pComponent->isComponentSystem())
        {
            forEachPort(dynamic_cast<hopsan::ComponentSystem*>(pComponent), function);
        }
    }
}


#endif // MODELUTILITIES_H
