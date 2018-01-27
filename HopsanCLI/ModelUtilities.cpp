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
//! @author FluMeS
//! @date   2012-05-30
//!
//! @brief Contains model specific helpfunctions for CLI
//!
//$Id$

#include <iostream>
#include <fstream>

#include "ModelUtilities.h"
#include "version_cli.h"
#include "CliUtilities.h"

#include "HopsanEssentials.h"

using namespace std;
using namespace hopsan;

void generateFullSubSystemHierarchyName(const ComponentSystem *pSys, HString &rFullSysName)
{
    if (pSys->getSystemParent())
    {
        generateFullSubSystemHierarchyName(pSys->getSystemParent(), rFullSysName);
        rFullSysName.append(pSys->getName()).append("$");
    }
    else
    {
        // Do not include top-level name in sub-system hierarchy
        rFullSysName.clear();
    }
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



void saveResults(ComponentSystem *pSys, const string &rFileName, const SaveResults howMany, string prefix, ofstream *pFile)
{
    bool doCloseFile=false;
    if (!pFile)
    {
        pFile = new ofstream;
        pFile->open(rFileName.c_str());
        if (!pFile->good())
        {
            printErrorMessage("Could not open: " + rFileName + " for writing!");
            delete pFile;
            return;
        }
        doCloseFile = true;
    }

    if (pSys)
    {
        // First save time vector for this system
        //! @todo alias a for time ? is that even posible
        if (howMany == Final)
        {
            *pFile << prefix.c_str() << "Time,,s," << pSys->getTime() << endl;
        }
        else if (howMany == Full)
        {
            vector<double> *pLogTimeVector = pSys->getLogTimeVector();
            if (pLogTimeVector->size() > 0)
            {
                *pFile << prefix.c_str() << "Time,,s";
                for (size_t t=0; t<pLogTimeVector->size(); ++t)
                {
                    *pFile << "," << (*pLogTimeVector)[t];//!< @todo what about precission
                }
                *pFile << endl;
            }
        }

        // Now save log data vectors for all subcomponents
        vector<HString> names = pSys->getSubComponentNames();
        for (size_t c=0; c<names.size(); ++c)
        {
            Component *pComp = pSys->getSubComponent(names[c]);
            if (pComp)
            {
                //cout << "comp: " << c << " of: " << names.size() << endl;
                if (pComp->isComponentSystem())
                {
                    // Save results for subsystem
                    saveResults(static_cast<ComponentSystem*>(pComp), rFileName, howMany, prefix+pComp->getName().c_str()+"$", pFile);
                }
                else
                {
                    vector<Port*> ports = pComp->getPortPtrVector();
                    for (size_t p=0; p<ports.size(); ++p)
                    {
                        //cout << "port: " << p << " of: " << ports.size() << endl;
                        Port *pPort = ports[p];
                        if (pPort->isMultiPort())
                        {
                            // Ignore multiports, not possible to determin what we want to log anyway
                            continue;
                        }
                        const vector<NodeDataDescription> *pVars = pPort->getNodeDataDescriptions();
                        if (pVars)
                        {
                            for (size_t v=0; v<pVars->size(); ++v)
                            {
                                HString fullname = prefix.c_str() + pComp->getName() + "#" + pPort->getName() + "#" + pVars->at(v).name;

                                if (howMany == Final)
                                {
                                    *pFile << fullname.c_str() << "," << pPort->getVariableAlias(v).c_str() << "," << pVars->at(v).unit.c_str();
                                    *pFile << "," << pPort->readNode(v) << endl; //!< @todo what about precission
                                }
                                else if (howMany == Full)
                                {
                                    // Only write something if data has been logged (skip ports that are not logged)
                                    // We assume that the data vector has been cleared
                                    if (pPort->getLogDataVectorPtr()->size() > 0)
                                    {
                                        *pFile << fullname.c_str() << "," << pPort->getVariableAlias(v).c_str() << "," << pVars->at(v).unit.c_str();
                                        //! @todo what about time vector
                                        vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();
                                        for (size_t t=0; t<pSys->getNumActuallyLoggedSamples(); ++t)
                                        {
                                            *pFile << "," << (*pLogData)[t][v];//!< @todo what about precission
                                        }
                                        *pFile << endl;
                                    }
                                }
                            }
                        }
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
            HString fullname = prefix.c_str() + pSystem->getName() + "#" + pSysParameters->at(p)->getName();
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
                        std::vector<std::string> vec2, nameVec, syshierarcy;
                        string componentName, parameterName;
                        splitStringOnDelimiter(lineVec[0], '$', vec2);

                        // Last of vec2 will contain the rest of the name filed (comp and param name)
                        int i;
                        for (i=0; i<int(vec2.size())-1; ++i)
                        {
                            syshierarcy.push_back(vec2[i]);
                        }

                        // Split last name part into comp and param name
                        splitStringOnDelimiter(vec2[i], '#', nameVec);
                        if (nameVec.size() == 2 || nameVec.size() == 3 )
                        {
                            if (nameVec.size() == 2)
                            {
                                componentName = nameVec[0];
                                parameterName = nameVec[1];
                            }
                            if (nameVec.size() == 3)
                            {
                                // Set component name and reset the parameter (startvalue) name
                                componentName = nameVec[0];
                                parameterName = nameVec[1]+"#"+nameVec[2];
                            }


                            // Dig down subsystem hiearchy
                            ComponentSystem *pParentSys = pSystem;
                            for (size_t s=1; s<syshierarcy.size(); ++s)
                            {
                                //! @todo what about first level (0), should we check that name is ok
                                ComponentSystem *pSubSys = pParentSys->getSubComponentSystem(syshierarcy[s].c_str());
                                if (!pSubSys)
                                {
                                    printErrorMessage("Subsystem: "+syshierarcy[s]+" could not be found in parent system: "+pParentSys->getName().c_str());
                                    pParentSys = 0;
                                    break;
                                }
                                else
                                {
                                    pParentSys = pSubSys;
                                }
                            }

                            // Set the parameter value if component is found
                            if (pParentSys)
                            {
                                Component *pComp = 0;
                                // If syshierarcy is empty then we are setting a parameter in the top-level system
                                if (syshierarcy.empty())
                                {
                                    pComp = pParentSys;
                                }
                                else
                                {
                                    pComp = pParentSys->getSubComponent(componentName.c_str());
                                }

                                if (pComp)
                                {
                                    // lineVec[1] should be parameter value
                                    //! @todo what about parameter alias
                                    bool ok = pComp->setParameterValue(parameterName.c_str(), lineVec[1].c_str());
                                    if (!ok)
                                    {
                                        printErrorMessage("Setting parameter: " + parameterName + " in component: " + componentName);
                                    }
                                }
                                else
                                {
                                    printErrorMessage("No component: " + componentName + " in system: " + pParentSys->getName().c_str() );
                                }
                            }
                        }
                        else
                        {
                            printErrorMessage(vec2[i] + " should be componentName#parameterName on line: " + line);
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

hopsan::Port* getPortWithFullName(hopsan::ComponentSystem *pRootSystem, const std::string &fullPortName)
{
    std::vector<std::string> nameParts;
    splitStringOnDelimiter(fullPortName, '#', nameParts);
    hopsan::ComponentSystem* pCurrentSystem = pRootSystem;
    hopsan::Component* pComponent = nullptr;
    for (const auto& namePart : nameParts)
    {
        // If component has been found, then look for the port
        if (pComponent)
        {
            return pComponent->getPort(namePart.c_str());
        }
        else
        {
            // First check if the component is a sub system, in which case the loop continues to the next part
            auto pSystemComponent = pCurrentSystem->getSubComponentSystem(namePart.c_str());
            if (pSystemComponent)
            {
                pCurrentSystem = pSystemComponent;
            }
            // Else check if this is an ordinary component
            else
            {
                pComponent = pCurrentSystem->getSubComponent(namePart.c_str());
                // If not found, maybe the name is that of an interface port (system port)
                if (!pComponent)
                {
                    return pCurrentSystem->getPort(namePart.c_str());
                }
            }
        }
    }
    return nullptr;
}
