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
//! @file   exe_utilities.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2019-06-03
//!
//! @brief Contains model specific helpfunctions for executable models
//!
//$Id$

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#include "exe_utilities.h"

#include "HopsanEssentials.h"

using namespace std;
using namespace hopsan;

void printWarningMessage(std::string msg)
{
    std::cout << "Warning: " << msg << "\n";
}

void printErrorMessage(std::string msg)
{
    std::cout << "Error: " << msg << "\n";
}

void splitStringOnDelimiter(const std::string &rString, const char delim, std::vector<std::string> &rSplitVector)
{
    rSplitVector.clear();
    string item;
    stringstream ss(rString);
    while(getline(ss, item, delim))
    {
        rSplitVector.push_back(item);
    }
}

void saveResults(ComponentSystem *pSys, const string &rFileName, const SaveResults howMany, const SaveDescriptions descriptions, string prefix, ofstream *pFile)
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
            *pFile << prefix.c_str() << "time,";
            if(descriptions == NameAliasUnit) {
                *pFile << ",s,";
            }
            *pFile << std::scientific << pSys->getTime() << endl;
        }
        else if (howMany == Full)
        {
            vector<double> *pLogTimeVector = pSys->getLogTimeVector();
            if (pLogTimeVector->size() > 0)
            {
                *pFile << prefix.c_str() << "time";
                if(descriptions == NameAliasUnit) {
                    *pFile << ",,s";
                }
                for (size_t t=0; t<pLogTimeVector->size(); ++t)
                {
                    *pFile << "," << std::scientific << (*pLogTimeVector)[t];
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
                vector<Port*> ports = pComp->getPortPtrVector();
                for (size_t p=0; p<ports.size(); ++p)
                {
                    //cout << "port: " << p << " of: " << ports.size() << endl;
                    Port *pPort = ports[p];
                    if (!pPort->isLoggingEnabled())
                    {
                        // Ignore ports that have logging disabled
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
                                *pFile << fullname.c_str();
                                if(descriptions == NameAliasUnit) {
                                    *pFile << "," << pPort->getVariableAlias(v).c_str() << "," << pVars->at(v).unit.c_str();
                                }
                                *pFile << "," << std::scientific << pPort->readNode(v) << endl;
                            }
                            else if (howMany == Full)
                            {
                                // Only write something if data has been logged (skip ports that are not logged)
                                // We assume that the data vector has been cleared
                                if (pPort->getLogDataVectorPtr()->size() > 0)
                                {
                                    *pFile << fullname.c_str();
                                    if(descriptions == NameAliasUnit) {
                                        *pFile << "," << pPort->getVariableAlias(v).c_str() << "," << pVars->at(v).unit.c_str();
                                    }
                                    //! @todo what about time vector
                                    vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();
                                    for (size_t t=0; t<pSys->getNumActuallyLoggedSamples(); ++t)
                                    {
                                        *pFile << "," << std::scientific << (*pLogData)[t][v];
                                    }
                                    *pFile << endl;
                                }
                            }
                        }
                    }
                }

                // Recurse into subsystems
                if (pComp->isComponentSystem())
                {
                    saveResults(static_cast<ComponentSystem*>(pComp), rFileName, howMany, descriptions, prefix+pComp->getName().c_str()+"$", pFile);
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
                        setParameter(lineVec[0], lineVec[1], pSystem);
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

bool startsWith(std::string str, std::string match)
{
    if(str.find(match) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool endsWith(std::string str, std::string match)
{
    if (str.length() >= match.length()) {
        return (0 == str.compare (str.length() - match.length(), match.length(), match));
    } else {
        return false;
    }
}

bool setParameter(string &rParName, string &rParValue, ComponentSystem *pSystem)
{
    std::string parName = rParName;

    //Replace '$' and '#' with ':' and '.'
    //(note that both '$' and '|' are problematic in terminal arguments on Linux)
    std::replace(parName.begin(), parName.end(), ':', '$');
    std::replace(parName.begin(), parName.end(), '.', '#');

    //Prepend system name of toplevel system
    std::string systemName = pSystem->getName().c_str();
    if(!startsWith(parName, systemName+"$")) {
        parName.insert(0,systemName+"$");
    }

    //Convert to short variable names (i.e. "Value" => y")
    if(endsWith(parName, "#y")) {
        parName.erase(parName.length()-2,2);
        parName.append("#Value");
    };

    std::vector<std::string> vec2, nameVec, syshierarcy;
    string componentName, parameterName;
    splitStringOnDelimiter(parName, '$', vec2);

    // Last of vec2 will contain the rest of the name filed (comp and param name)
    int i;
    for (i=0; i<int(vec2.size())-1; ++i) {
        syshierarcy.push_back(vec2[i]);
    }

    // Split last name part into comp and param name
    splitStringOnDelimiter(vec2[i], '#', nameVec);
    if (nameVec.size() == 2 || nameVec.size() == 3 ) {
        if (nameVec.size() == 2) {
            componentName = nameVec[0];
            parameterName = nameVec[1];
        }
        if (nameVec.size() == 3) {
            // Set component name and reset the parameter (startvalue) name
            componentName = nameVec[0];
            parameterName = nameVec[1]+"#"+nameVec[2];
        }

        // Dig down subsystem hiearchy
        ComponentSystem *pParentSys = pSystem;
        for (size_t s=1; s<syshierarcy.size(); ++s) {
            //! @todo what about first level (0), should we check that name is ok
            ComponentSystem *pSubSys = pParentSys->getSubComponentSystem(syshierarcy[s].c_str());
            if (!pSubSys) {
                printErrorMessage("Subsystem: "+syshierarcy[s]+" could not be found in parent system: "+pParentSys->getName().c_str());
                return false;
            }
            else {
                pParentSys = pSubSys;
            }
        }

        // Set the parameter value if component is found
        if (pParentSys) {
            Component *pComp = 0;
            // If syshierarcy is empty then we are setting a parameter in the top-level system
            if (syshierarcy.empty()) {
                pComp = pParentSys;
            }
            else {
                pComp = pParentSys->getSubComponent(componentName.c_str());
            }

            if (pComp) {
                bool ok = pComp->setParameterValue(parameterName.c_str(), rParValue.c_str());
                std::cout << "Setting parameter: " << pComp->getName().c_str() << "." << parameterName << " = " << rParValue << "\n";
                if (!ok) {
                    std::cout << "Failed!\n";
                    return false;
                }
            }
            else {
                printErrorMessage("No component: " + componentName + " in system: " + pParentSys->getName().c_str() );
                return false;
            }
        }
    }
    else {
        std::cout << "Unknown error!\n";
        return false;
    }

    return true;
}

void printHelpText(ComponentSystem *pSystem)
{
    std::cout << "This is a stand-alone executable Hopsan model.\n";
    std::cout << "Model name: " << pSystem->getName().c_str() << "\n";
    std::cout << "\n";
    std::cout << "Usage: " << pSystem->getName().c_str() << " [FLAG]\n";
    std::cout << "Usage: " << pSystem->getName().c_str() << " [OPTIONS]\n";
    std::cout << "\n";
    std::cout << "Flags:\n";
    std::cout << "  -h, --help            Display this help text\n";
    std::cout << "  -s, --simulate        Run a simulation\n";
    std::cout << "  -p, --parameters      List all tunable model parameters\n";
    std::cout << "\n";
    std::cout << "Options (cannot be used together with flags):\n";
    std::cout << "  start=[real]                Simulation start time\n";
    std::cout << "  step=[real]                 Simulation step size\n";
    std::cout << "  stop=[real]                 Simulation stop time\n";
    std::cout << "  samples=[integer]           Number of log samples\n";
    std::cout << "  results=full/final          Print full results or final results only\n";
    std::cout << "  samples=[integer]           Number of log samples\n";
    std::cout << "  transpose=true/false        Transpose to column-wise CSV file\n";
    std::cout << "  descriptions=all/namesonly  Print names, alias and unit or only name\n";
    std::cout << "  progress=true/false         Enable or disable simulation progress output\n";
    std::cout << "  parameterfile=[filename]    Specify parameter input file\n";
    std::cout << "  configfile=[filename]       Specify configuration file\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  >> " << pSystem->getName().c_str() << " start=0 step=0.001 stop=3\n";
    std::cout << "  >> " << pSystem->getName().c_str() << " samples=1000 transpose=true descriptions=namesonly\n";
    std::cout << "  >> " << pSystem->getName().c_str() << " engine.rpm=2000 results=final\n";
}

void appendParameters(ComponentSystem *pSystem, std::vector<HString> &allParameters, HString prefix="")
{
    std::vector<HString> sysParameters;
    pSystem->getParameterNames(sysParameters);
    for(HString &par : sysParameters) {
        std::string temp = prefix.c_str();
        temp.append(pSystem->getName().c_str());
        temp.append("#");
        temp.append(par.c_str());
        par = temp.c_str();
    }
    allParameters.insert(allParameters.end(), sysParameters.begin(), sysParameters.end());

    for(Component *pComp : pSystem->getSubComponents()) {
        if(pComp->isComponentSystem()) {
            prefix.append(pComp->getName().c_str());
            prefix.append("$");
            appendParameters(reinterpret_cast<ComponentSystem*>(pComp), allParameters);
        }
        std::vector<HString> compParameters;
        pComp->getParameterNames(compParameters);
        for(HString &par : compParameters) {
            std::string temp = prefix.c_str();
            temp.append(pComp->getName().c_str());
            temp.append("#");
            temp.append(par.c_str());
            par = temp.c_str();
        }
        allParameters.insert(allParameters.end(), compParameters.begin(), compParameters.end());
    }
}

void printParameters(ComponentSystem *pSystem)
{
    std::vector<HString> allParameters;
    appendParameters(pSystem, allParameters);
    std::sort(allParameters.begin(), allParameters.end());

    std::cout << "Model parameters:\n";
    for(HString &par : allParameters) {
        par.replace("$", ":");
        par.replace("#", ".");
        std::cout << "  " << par.c_str() << "\n";
    }
}

void readConfigFile(std::string &filePath, Options &options) {
    std::ifstream file;
    file.open(filePath.c_str());
    if (file.is_open()) {
        std::string line;
        while (file.good()) {
            getline(file, line);
            std::string name = line.substr(0, line.find("="));
            std::string value = line.substr(line.find("=")+1, line.size()-1);
            options.set(name, value);
        }
    }
    else {
        std::cout << "Error: Unable to read config file: " << filePath << "\n";
        exit(1);
    }
}

//! @brief Prints all waiting messages
//! @param[in] printDebug Should debug messages also be printed
void printWaitingMessages(hopsan::HopsanEssentials& hopsanCore, bool printDebug, bool silent)
{
    if(silent) return;

    hopsan::HString msg, type, tag;
    while (hopsanCore.checkMessage() > 0) {
        hopsanCore.getMessage(msg,type,tag);
        if (type == "debug") {
            if (printDebug) {
                cout << msg.c_str() << endl;
            }
        }
        else {
            cout << msg.c_str() << endl;
        }
    }
}
