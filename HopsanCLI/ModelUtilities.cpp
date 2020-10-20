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
//! @file   ModelUtilities.cpp
//! @brief Contains model specific helpfunctions for CLI
//!

#include <iostream>
#include <fstream>
#include <algorithm>

#include "ModelUtilities.h"
#include "version_cli.h"
#include "CliUtilities.h"

#include "HopsanEssentials.h"
#include "HopsanTypes.h"

#ifdef USEHDF5
#include "hopsanhdf5exporter.h"
#endif

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

HString generateFullPortVariableName(const Port *pPort, const size_t dataId)
{
   HString fullName;
   const NodeDataDescription *pND = pPort->getNodeDataDescription(dataId);
   if (pND)
   {
       Component *pComp = pPort->getComponent();
       ComponentSystem *pSys = pComp->getSystemParent();
       if (pSys)
       {
           generateFullSubSystemHierarchyName(pSys, fullName);
       }

       fullName.append(pComp->getName()).append("#").append(pPort->getName()).append("#").append(pND->name);
   }
   return fullName;
}


//! @brief Helpfunction to print timestep info for a system
//! @param[in] pSystem The system to print info for
void printTsInfo(const ComponentSystem* pSystem)
{
    cout << "Ts: " << pSystem->getDesiredTimeStep() << " InheritTs: " << pSystem->doesInheritTimestep();
}

//! @brief Print system parameters in a system
//! @param[in] pSystem The system to print info for
void printSystemParams(ComponentSystem* pSystem)
{
    const vector<ParameterEvaluator*> *pParams = pSystem->getParametersVectorPtr();
    for (size_t i=0; i<pParams->size(); ++i)
    {
        cout << " SysParam: " << pParams->at(i)->getName().c_str() << "=" << pParams->at(i)->getValue().c_str();
    }
}

//! @brief Print component hierarcy in a system
//! @param[in] pSystem The system to print info for
//! @param[in] prefix Text to add before printout
//! @param[in] doPrintTsInfo Should timestep info be included
//! @param[in] doPrintSystemParams Should system parameters be printed
void printComponentHierarchy(ComponentSystem *pSystem, std::string prefix,
                             const bool doPrintTsInfo,
                             const bool doPrintSystemParams)
{
    if (pSystem)
    {
        cout << prefix << pSystem->getName().c_str() << ", ";
        if (doPrintTsInfo)
        {
            printTsInfo(pSystem);
        }

        if (doPrintSystemParams)
        {
            cout << " ";
            printSystemParams(pSystem);
        }

        cout << endl;


        prefix.append("  ");
        vector<HString> names = pSystem->getSubComponentNames();
        for (size_t i=0; i<names.size(); ++i)
        {
            if ( pSystem->getSubComponent(names[i])->isComponentSystem() )
            {
                printComponentHierarchy(pSystem->getSubComponentSystem(names[i]), prefix, doPrintTsInfo, doPrintSystemParams);
            }
            else
            {
                cout << prefix << names[i].c_str() << endl;
            }
        }
    }
}



void transposeCSVresults(const std::string &rFileName)
{
    ifstream infile(rFileName.c_str());
    if ( infile.good() )
    {
        vector< vector<string> > linesVec;
        vector<string> lineVec;
        // Read all lines into string vectors
        while(!infile.eof())
        {
            string line;
            getline(infile, line);

            // Split on delimiter
            splitStringOnDelimiter(line, ',', lineVec);

            // Store line vector
            if (lineVec.size() > 0)
            {
                linesVec.push_back(lineVec);
                lineVec.clear();

                // Clear data but reserve memory for next run
                size_t n = lineVec.size();
                lineVec.clear();
                lineVec.reserve(n);
            }
        }
        infile.close();

        // Write back transposed
        ofstream ofile(rFileName.c_str());
        if ( ofile.good() )
        {
            size_t rows = linesVec.size();
            if (rows > 0)
            {
                // Assumes same nr cols on each row
                size_t cols = linesVec[0].size();
                for (size_t c=0; c<cols; ++c)
                {
                    ofile << linesVec[0][c];
                    //cout << linesVec[0][c];
                    for (size_t r=1; r<rows; ++r)
                    {
                        ofile << "," <<  linesVec[r][c];
                        //cout << ", " <<  linesVec[r][c];
                    }
                    ofile << endl;
                    //cout << endl;
                }
            }
        }
        ofile.close();
    }
}

void exportParameterValuesToCSV(const std::string &rFileName, hopsan::ComponentSystem* pSystem, string prefix, ofstream *pFile)
{
    bool doCloseFile=false;
    if (!pFile)
    {
        pFile = new ofstream;
        pFile->open(rFileName.c_str());
        if (!pFile->good())
        {
            printErrorMessage("Could not open: " + rFileName + " for writing!");
            return;
            delete pFile;
        }
        doCloseFile = true;
    }

    if (pSystem)
    {
        // Handle own system parameters
        const std::vector<ParameterEvaluator*> *pSysParameters =  pSystem->getParametersVectorPtr();
        for (size_t p=0; p<pSysParameters->size(); ++p)
        {
            //! @todo what about alias name
            HString fullSelfName = prefix.empty() ? "self" : prefix.c_str();
            HString fullname = fullSelfName + "#" + pSysParameters->at(p)->getName();
            *pFile << fullname.c_str() << "," << pSysParameters->at(p)->getValue().c_str() << endl;
        }

        // Now handle subcomponent parameters
        vector<HString> names = pSystem->getSubComponentNames();
        for (size_t c=0; c<names.size(); ++c)
        {
            Component *pComp = pSystem->getSubComponent(names[c]);
            if (pComp)
            {
                if (pComp->isComponentSystem())
                {
                    exportParameterValuesToCSV(rFileName, static_cast<ComponentSystem*>(pComp), prefix+pComp->getName().c_str()+"$", pFile);
                }
                else
                {
                    const std::vector<ParameterEvaluator*> *pParameters =  pComp->getParametersVectorPtr();
                    for (size_t p=0; p<pParameters->size(); ++p)
                    {
                        //! @todo what about alias name
                        HString parname = pParameters->at(p)->getName();
                        HString fullname = prefix.c_str() + pComp->getName() + "#" + parname;
                        *pFile << fullname.c_str() << "," << pParameters->at(p)->getValue().c_str() << endl;
                    }
                }
            }
        }
    }

    if (doCloseFile)
    {
        pFile->close();
        delete pFile;
    }

}

//! @todo should we use CSV parser instead?
void importParameterValuesFromCSV(const std::string filePath, hopsan::ComponentSystem* pSystem)
{
    if (pSystem)
    {
        std::ifstream file;
        file.open(filePath.c_str());
        if ( file.is_open() )
        {
            std::vector<std::string> lineVec;
            std::string line;
            while ( file.good() )
            {
                getline(file, line);
                if (*line.begin() != '#')
                {
                    // Split on delimiter
                    splitStringOnDelimiter(line, ',', lineVec);

                    // Parse line vector
                    if (lineVec.size() == 2)
                    {
                        std::vector<std::string> nameParts;
                        string fullComponentName, parameterName;
                        splitStringOnDelimiter(lineVec[0], '#', nameParts);

                        // Split last name part into fullcomponent and parameter#value name
                        if (nameParts.size() == 2 || nameParts.size() == 3 ) {
                            if (nameParts.size() == 2) {
                                fullComponentName = nameParts[0];
                                parameterName = nameParts[1];
                            }
                            if (nameParts.size() == 3) {
                                // Set component name and reset the parameter (startvalue) name
                                fullComponentName = nameParts[0];
                                parameterName = nameParts[1]+"#"+nameParts[2];
                            }


                            hopsan::Component* pComponent;
                            if (fullComponentName == "self") {
                                pComponent = pSystem;
                            }
                            else {
                                pComponent = getComponentWithFullName(pSystem, fullComponentName);
                            }
                            if (pComponent) {
                                // lineVec[1] should be the parameter value
                                //! @todo what about parameter alias
                                bool ok = pComponent->setParameterValue(parameterName.c_str(), lineVec[1].c_str());
                                if (!ok) {
                                    printErrorMessage("Setting parameter: " + parameterName + " in component: " + fullComponentName);
                                }
                            }
                        }
                        else {
                            printErrorMessage(lineVec[0] + " should be FullComponentName#ParameterName on line: " + line);
                        }
                    }
                    else
                    {
                        // Print error for non-empty lines
                        if (lineVec.size() > 0)
                        {
                            printWarningMessage(string("Wrong line format: ") + line);
                        }
                    }
                }
            }
            file.close();
        }
        else
        {
            printErrorMessage(string("Could not open file: ")+filePath);
        }
    }
}


//! @brief Read data on which nodes to save after simualtion from text file
//! @param[in] filePath The file to read from
//! @param[out] rComps The component names
//! @param[out] rPorts The port names
void readNodesToSaveFromTxtFile(const std::string filePath, std::vector<std::string> &rComps, std::vector<std::string> &rPorts)
{
    rComps.clear();
    rPorts.clear();
    std::string line;
    std::ifstream file;
    file.open(filePath.c_str());
    if ( file.is_open() )
    {
        while ( file.good() )
        {
            getline(file, line);
            cout << "line: " << line << endl;
            if (*line.begin() != '#')
            {
                std::string comp, port;
                size_t sep;

                sep = line.find(';');
                comp = line.substr(0, sep);
                port = line.substr(sep+1);

                rComps.push_back(comp);
                rPorts.push_back(port);
            }
        }
        file.close();
    }
    else
    {
        printErrorMessage("Could not open file: " + filePath);
    }
}

Component *getComponentWithFullName(ComponentSystem *pRootSystem, const string &fullComponentName)
{
    std::vector<std::string> nameParts;
    splitStringOnDelimiter(fullComponentName, '$', nameParts);

    // Search into subsystems
    hopsan::ComponentSystem* pCurrentSystem = pRootSystem;
    for (size_t i=0; i<nameParts.size()-1; ++i) {
        // First check if the component is a sub system, in which case the loop continues to the next part
        auto pSystemComponent = pCurrentSystem->getSubComponentSystem(nameParts[i].c_str());
        if (pSystemComponent) {
            pCurrentSystem = pSystemComponent;
        }
        else {
            printErrorMessage("Subsystem: "+nameParts[i]+" could not be found in parent system: "+pCurrentSystem->getName().c_str());
            return nullptr;
        }
    }

    // Now lookup the component
    hopsan::Component* pComponent = pCurrentSystem->getSubComponent(nameParts.back().c_str());
    if (pComponent) {
        return pComponent;
    }
    else {
        printErrorMessage("No component: " + nameParts.back() + " in system: " + pCurrentSystem->getName().c_str() );
        return nullptr;
    }
}

hopsan::Port* getPortWithFullName(hopsan::ComponentSystem *pRootSystem, const std::string &fullPortName)
{
    std::vector<std::string> nameParts;
    splitStringOnDelimiter(fullPortName, '#', nameParts);

    hopsan::Component* pComponent = getComponentWithFullName(pRootSystem, nameParts.front());
    if (pComponent) {
        std::string portName;
        if (nameParts.size() == 2 || nameParts.size() == 3 ) {
            portName = nameParts[1];
            // Ignore ValuePart numParts[2] if it is pressent
        }
        else {
            printErrorMessage("A portname on the format ComponentName#PortName could not be found: " + fullPortName);
        }

        return pComponent->getPort(portName.c_str());
    }
    return nullptr;
}


//! @brief Save results to HDF5 format
//! @param [in] pRootSystem Pointer to component system
//! @param [in] rFileName File name for output file
//! @param [in] includeFilter list of full port names or variables names to include (excluding all others)
//! @param [in] howMany Specifies if all results or only final values should be saved
void saveResultsToHDF5(ComponentSystem *pRootSystem, const string &rFileName, const std::vector<string>& includeFilter, const SaveResults howMany)
{
#ifdef USEHDF5
    if(!pRootSystem) {
        return;
    }
    HopsanHDF5Exporter exporter(rFileName.c_str(), pRootSystem->getName().c_str(), std::string("HopsanCLI "+std::string(HOPSANCLIVERSION)).c_str());

    auto addTimeVariable = [&exporter, howMany](ComponentSystem* pSystem) {
        vector<double> *pLogTimeVector = pSystem->getLogTimeVector();
        const size_t numLoggedSamples = pSystem->getNumActuallyLoggedSamples();
        if (numLoggedSamples > 0) {
            HVector<double> timeVector;
            if(howMany == Full) {
                timeVector.assign_from(pLogTimeVector->data(), numLoggedSamples);
            }
            else {
                timeVector.append((*pLogTimeVector)[numLoggedSamples-1]);
            }
            HString parentSystemNames = generateFullSubSystemHierarchyName(pSystem,".", false);
            exporter.addVariable(parentSystemNames, "", "","Time","","s","Time",timeVector);
        }
    };

    auto addVariable = [&exporter, howMany](const ComponentSystem* pSystem, const Component* pComponent, const Port* pPort, size_t variableIndex) {
        const vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();
        const size_t numLoggedSamples = pSystem->getNumActuallyLoggedSamples();
        if( (pLogData != nullptr) && !pLogData->empty() && (numLoggedSamples > 0)) {
            HVector<double> dataVector;
            if(howMany == Full) {
                dataVector.reserve(numLoggedSamples);
                for (size_t t=0; t < numLoggedSamples; ++t) {
                    dataVector.append((*pLogData)[t][variableIndex]);
                }
            }
            else {
                dataVector.append((*pLogData)[numLoggedSamples-1][variableIndex]);
            }

            HString parentSystemNames = generateFullSubSystemHierarchyName(pSystem,".", false);
            const NodeDataDescription& variable = *pPort->getNodeDataDescription(variableIndex);

            exporter.addVariable(parentSystemNames, pComponent->getName(), pPort->getName(), variable.name, pPort->getVariableAlias(variableIndex).c_str(),
                                 variable.unit, variable.quantity, dataVector);
        }
    };

    saveResultsTo(pRootSystem, includeFilter, addTimeVariable, addVariable);

    bool writeOK = exporter.writeToFile();
    if (!writeOK) {
        printErrorMessage(("Failure when writing HDF5 file: "+exporter.getLastError()).c_str());
    }
#else
    printErrorMessage("HopsanCLI was built without HDF5 support");
#endif
}

//! @brief Save results to HDF5 format
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
            printErrorMessage("Could not open: " + rFileName + " for writing!");
        }

        outfile.close();
    }
}


