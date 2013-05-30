/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   cliFunctions.cpp
//! @author FluMeS
//! @date   2012-05-30
//!
//! @brief Contains helpfunctions for CLI
//!
//$Id$

#include "cliFunctions.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

#include "HopsanEssentials.h"
#include "ComponentUtilities/CSVParser.h"

#include "hopsan_rapidxml.hpp"

#ifdef WIN32
#include "windows.h"
#endif

using namespace std;
using namespace hopsan;

extern HopsanEssentials gHopsanCore;

// ===== Help functions =====

//! @brief Helpfunction that splits a full path into basepath and filename
//! @note Assumes that dir separator is forward slash /
void splitFilePath(const std::string fullPath, std::string &rBasePath, std::string &rFileName)
{
    size_t pos = fullPath.rfind('/');
    // If not found try a windows backslash instead
    if (pos == std::string::npos)
    {
        pos = fullPath.rfind('\\');
    }
    if (pos != std::string::npos)
    {
        rBasePath = fullPath.substr(0, pos+1) ;
        rFileName = fullPath.substr(pos+1);
    }
    else
    {
        rBasePath = "";
        rFileName = fullPath;
    }
}

//! @brief Helpfunction that splits a filename into basename and extesion
void splitFileName(const std::string fileName, std::string &rBaseName, std::string &rExt)
{
    size_t pos = fileName.rfind('.');
    if (pos != std::string::npos)
    {
        rBaseName = fileName.substr(0, pos) ;
        rExt = fileName.substr(pos+1);
    }
    else
    {
        rExt = "";
        rBaseName = fileName;
    }
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

// ===== Print functions =====
//! @brief Prints all waiting messages
//! @param [in] printDebug Should debug messages also be printed
void printWaitingMessages(const bool printDebug)
{
    hopsan::HString msg, type, tag;
    while (gHopsanCore.checkMessage() > 0)
    {
        gHopsanCore.getMessage(msg,type,tag);
        if ( (type == "error") || ( type == "fatal") )
        {
            setTerminalColor(Red);
            cout << msg.c_str() << endl;
        }
        else if (type == "warning")
        {
            setTerminalColor(Yellow);
            cout << msg.c_str() << endl;
        }
        else if (type == "debug")
        {
            if (printDebug)
            {
                setTerminalColor(Blue);
                cout << msg.c_str() << endl;
            }
        }
        else
        {
            setTerminalColor(White);
            cout << msg.c_str() << endl;
        }
    }
    setTerminalColor(Reset);
}

//! @brief Prints a message with red color (resets color to defaul after)
//! @param [in] rError The error message
void printErrorMessage(const std::string &rError)
{
    setTerminalColor(Red);
    cout << "Error: " << rError << endl;
    setTerminalColor(Reset);
}

//! @brief Prints a message with yellow color (resets color to defaul after)
//! @param [in] rWarning The warning message
void printWarningMessage(const std::string &rWarning)
{
    setTerminalColor(Yellow);
    cout << "Warning: " << rWarning << endl;
    setTerminalColor(Reset);
}

//! @brief Prints a message with green color (resets color to defaul after)
//! @param [in] rMessage The message
void printColorMessage(const ColorsEnumT color, const std::string &rMessage)
{
    setTerminalColor(color);
    cout << rMessage << endl;
    setTerminalColor(Reset);
}

//! @brief Helpfunction to print timestep info for a system
//! @param [in] pSystem The system to print info for
void printTsInfo(const ComponentSystem* pSystem)
{
    cout << "Ts: " << pSystem->getDesiredTimeStep() << " InheritTs: " << pSystem->doesInheritTimestep();
}

//! @brief Print system parameters in a system
//! @param [in] pSystem The system to print info for
void printSystemParams(ComponentSystem* pSystem)
{
    const vector<Parameter*> *pParams = pSystem->getParametersVectorPtr();
    for (size_t i=0; i<pParams->size(); ++i)
    {
        cout << " SysParam: " << pParams->at(i)->getName().c_str() << "=" << pParams->at(i)->getValue().c_str();
    }
}

//! @brief Print component hierarcy in a system
//! @param [in] pSystem The system to print info for
//! @param [in] prefix Text to add before printout
//! @param [in] doPrintTsInfo Should timestep info be included
//! @param [in] doPrintSystemParams Should system parameters be printed
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

//! @brief Changes color on console output
//! @param color Color number (0-15)
//! @todo Need yellow color also
void setTerminalColor(const ColorsEnumT color)
{
#ifdef WIN32
    WORD c;
    switch (color)
    {
    case Red:
        c = FOREGROUND_INTENSITY | FOREGROUND_RED;
        break;
    case Green:
        c = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
        break;
    case Yellow:
        c = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED;
        break;
    case Blue:
        c = FOREGROUND_INTENSITY | FOREGROUND_BLUE;
        break;
    case White:
    default:
        c = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    }

    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hcon,c);
#else
    string c;
    switch (color)
    {
    case Red:
        c = "\e[0;91m";
        break;
    case Green:
        c = "\e[0;92m";
        break;
    case Yellow:
        c = "\e[0;93m";
        break;
    case Blue:
        c = "\e[0;94m";
        break;
    case White:
    default:
        c = "\e[0m";
    }

    cout << c;
#endif
}

// ===== Save functions =====
//! @brief Saved specified node data to text file
//! @param [in] pSys The system in whcih the component live
//! @param [in] compName The name of the component
//! @param [in] portName The name of the port in which the node exist
//! @param [in] fileName The name of the output file
void saveNodeDataToFile(ComponentSystem* pSys, const string compName, const string portName, const string fileName)
{
    if (pSys)
    {
        Component* pComp = pSys->getSubComponentOrThisIfSysPort(compName);
        if (pComp)
        {
            Port* pPort = pComp->getPort(portName);
            if (pPort)
            {
                pPort->saveLogData(fileName);
                return; //Abort function
            }
            printErrorMessage("Could not find portName: " + portName);
            return; //Abort function
        }
        printErrorMessage("Could not find compName: " + compName);
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
        prefix = prefix + pSys->getName().c_str() + "$";
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
                    saveResults(static_cast<ComponentSystem*>(pComp), rFileName, howMany, prefix, pFile);
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
                            continue;
                        }
                        const vector<NodeDataDescription> *pVars = pPort->getNodeDataDescriptions();
                        if (pVars)
                        {
                            for (size_t v=0; v<pVars->size(); ++v)
                            {
                                HString fullname = prefix + pComp->getName() + "#" + pPort->getName() + "#" + pVars->at(v).name;

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
        const std::vector<Parameter*> *pSysParameters =  pSystem->getParametersVectorPtr();
        for (size_t p=0; p<pSysParameters->size(); ++p)
        {
            //! @todo what about alias name
            HString fullname = prefix + pSystem->getName() + "#" + toStdString(pSysParameters->at(p)->getName());
            *pFile << fullname.c_str() << "," << pSysParameters->at(p)->getValue().c_str() << endl;
        }

        // Now handle subcomponent parameters
        prefix = prefix + pSystem->getName().c_str() + "$";
        vector<HString> names = pSystem->getSubComponentNames();
        for (size_t c=0; c<names.size(); ++c)
        {
            Component *pComp = pSystem->getSubComponent(names[c]);
            if (pComp)
            {
                if (pComp->isComponentSystem())
                {
                    exportParameterValuesToCSV(rFileName, static_cast<ComponentSystem*>(pComp), prefix, pFile);
                }
                else
                {
                    const std::vector<Parameter*> *pParameters =  pComp->getParametersVectorPtr();
                    for (size_t p=0; p<pParameters->size(); ++p)
                    {
                        //! @todo what about alias name
                        HString fullname = prefix + pComp->getName() + "#" + pParameters->at(p)->getName();
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

// ===== Load Functions =====

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
                        if (nameVec.size() == 2)
                        {
                            componentName = nameVec[0];
                            parameterName = nameVec[1];

                            // Dig down subsystem hiearchy
                            ComponentSystem *pParentSys = pSystem;
                            for (size_t s=1; s<syshierarcy.size(); ++s)
                            {
                                //! @todo what about first level (0), should we check that name is ok
                                ComponentSystem *pSubSys = pParentSys->getSubComponentSystem(syshierarcy[s]);
                                if (!pSubSys)
                                {
                                    printErrorMessage(string("Subsystem: ") + syshierarcy[s] + string(" could not be found in parent system: ") + toStdString(pParentSys->getName()));
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
                                Component *pComp = pParentSys->getSubComponent(componentName);
                                if (pComp)
                                {
                                    // lineVec[1] should be parameter value
                                    //! @todo what about parameter alias
                                    bool ok = pComp->setParameterValue(parameterName, lineVec[1]);
                                    if (!ok)
                                    {
                                        printErrorMessage("Setting parameter: " + parameterName + " in component: " + componentName);
                                    }
                                }
                                else
                                {
                                    printErrorMessage("No component: " + componentName + " in system: " + toStdString(pParentSys->getName()) );
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

//! @brief Read the paths of external liubs from a text file
//! @param [in] filePath The file to read from
//! @param [out] rExtLibFileNames A vector with paths to the external libs to load
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames)
{
    rExtLibFileNames.clear();
    std::string line;
    std::ifstream file;
    file.open(filePath.c_str());
    if ( file.is_open() )
    {
        while ( file.good() )
        {
            getline(file, line);
            if (*line.begin() != '#')
            {
                rExtLibFileNames.push_back(line);
            }
        }
        file.close();
    }
    else
    {
        printErrorMessage(string("Could not open externalLibsToLoadFile: ") + filePath );
    }
}

//! @brief Read data on which nodes to save after simualtion from text file
//! @param [in] fielPath The file to read from
//! @param [out] rComps The component names
//! @param [out] rPorts The port names
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

// ===== Compare functions =====
//! @brief Compares a vector with a reference vector
//! @param vec Vector to compare
//! @param ref Reference vector
//! @param Tolereance (%) for acceptance
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol)
{
    if(rVec.size() != rRef.size())
    {
        printErrorMessage("compareVectors() Size mismatch!");
        return false;
    }

    for(size_t i=0; i<rVec.size(); ++i)
    {
        if(fabs(1-rVec.at(i)/rRef.at(i)) > tol && !(rVec.at(i) < 1e-100 && rRef.at(i) < 1e-10))
        {
            //cout << "Test failed: comparing " << vec.at(i) << " with " << ref.at(i) << " at index " << i << endl;
            return false;
        }
    }
    return true;
}


//! @brief Performs a unit test on a model
//! @param modelName Name of test model
bool performModelTest(const std::string hvcFilePath)
{
    // Figure out basepath and basename
    string basepath, basename, filename, ext;
    splitFilePath(hvcFilePath, basepath, filename);
    splitFileName(filename, basename, ext);
    //cout << basepath << " :: " << filename << " :: " << basename << " :: " << ext << endl;

    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> hmfFile(hvcFilePath.c_str());
        doc.parse<0>(hmfFile.data());
        rapidxml::xml_node<> *pRootNode = doc.first_node();

        //Check for correct root node name
        //! @todo should check version also
        if (strcmp(pRootNode->name(), "hopsanvalidationconfiguration")!=0)
        {
            printErrorMessage(hvcFilePath  + " Has wrong root tag name: " + string(pRootNode->name()));
            return false;
        }

        rapidxml::xml_node<> *pValidationNode = pRootNode->first_node("validation");
        if (!pValidationNode)
        {
            printErrorMessage("No validation node found in xml");
            return false;
        }
        while (pValidationNode != 0)
        {
            // Run each validation
            string modelfile = readStringNodeValue(pValidationNode->first_node("modelfile"), "");
            string parameterset = readStringNodeValue(pValidationNode->first_node("parameterset"), "");

            // If no modelfile was given use one with the same basename
            if (modelfile.empty())
            {
                modelfile = basepath + basename + ".hmf";
            }
            else
            {
                // Assumes that modelfile path was relaitve in xml
                modelfile = basepath + modelfile;
            }

            rapidxml::xml_node<> *pComponentNode, *pPortNode, *pVariableNode;
            pComponentNode = pValidationNode->first_node("component");
            if (!pComponentNode)
            {
                printErrorMessage("No component node found in xml");
                return false;
            }
            while (pComponentNode != 0)
            {
                string compName = readStringAttribute(pComponentNode, "name", "_noname_");
                pPortNode = pComponentNode->first_node("port");
                if (!pPortNode)
                {
                    printErrorMessage("No port node found in xml");
                    return false;
                }
                while (pPortNode != 0)
                {
                    string portName = readStringAttribute(pPortNode, "name", "_noname_");
                    string csvfile = readStringNodeValue(pPortNode->first_node("csvfile"), "");
                    pVariableNode = pPortNode->first_node("variable");
                    if (!pVariableNode)
                    {
                        printErrorMessage("No variable node found in xml");
                        return false;
                    }
                    while (pVariableNode != 0)
                    {
                        string varname = readStringAttribute(pVariableNode, "name", "_noname_");
                        csvfile = readStringNodeValue(pVariableNode->first_node("csvfile"), csvfile); //Do we have variable specific csv override
                        if (csvfile.empty())
                        {
                            // If no csvfile was given use one with the same basename
                            csvfile = basepath + basename + ".csv";
                        }
                        else
                        {
                            // Assumes that csvfile path was relaitve in xml
                            csvfile = basepath + csvfile;
                        }

                        const int column = readIntNodeValue(pVariableNode->first_node("column"), 1);
                        const double tolerance = readDoubleNodeValue(pVariableNode->first_node("tolerance"), 0.01);

                        // ----- Perform test -----
                        vector<double> vRef, vSim1, vSim2, vTime;

                        // Load reference data curve
                        //! @todo should not reload if same as already laoded
                        bool success=false;
                        CSVParser refData(success, csvfile, '\n', '"');
                        if(!success)
                        {
                            printErrorMessage("Unable to initialize CSV file: " + csvfile + " : " + refData.getErrorString());
                            return false;
                        }

                        double startTime=0, stopTime=1;
                        ComponentSystem* pRootSystem = gHopsanCore.loadHMFModel(modelfile.c_str(), startTime, stopTime);

                        if (pRootSystem!=0)
                        {
                            //! @todo maybe use simulation handler object
                            //First simulation
                            if (!pRootSystem->checkModelBeforeSimulation())
                            {
                                printWaitingMessages(false);
                                printErrorMessage("checkModelBeforeSimulation() failed, Simulation aborted!");
                                return false;
                            }

                            if (pRootSystem->initialize(startTime, stopTime))
                            {
                                pRootSystem->simulate(stopTime);
                            }
                            else
                            {
                                printWaitingMessages(false);
                                printErrorMessage("Initialize failed, Simulation aborted!");
                                return false;
                            }
                            pRootSystem->finalize();

                            //copy the data
                            Component* pComp = pRootSystem->getSubComponent(compName);
                            if (!pComp)
                            {
                                printErrorMessage("No such component name: " + compName);
                                return false;
                            }
                            Port *pPort = pComp->getPort(portName);
                            if (!pPort)
                            {
                                printErrorMessage("No such port name: " + portName + " in component: " + compName);
                                return false;
                            }

                            //! @todo with better access function in core we could avoid copying data and work directly with the data stored
                            vTime = *pPort->getLogTimeVectorPtr();
                            int dataId = pPort->getNodeDataIdFromName(varname);
                            if (dataId < 0)
                            {
                                printErrorMessage("No such varaiable name: " + varname + " in: " + toStdString(pPort->getNodeType()));
                                return false;
                            }

                            for(size_t i=0; i<vTime.size(); ++i)
                            {
                                vSim1.push_back(pRootSystem->getSubComponent(compName)->getPort(portName)->getLogDataVectorPtr()->at(i).at(dataId));
                            }

                            //Second simulation
                            if (pRootSystem->initialize(startTime, stopTime))
                            {
                                pRootSystem->simulate(stopTime);
                            }
                            else
                            {
                                printWaitingMessages(false);
                                printErrorMessage("Initialize failed, Simulation aborted!");
                                return false;
                            }
                            pRootSystem->finalize();

                            for(size_t i=0; i<vTime.size(); ++i)
                            {
                                vSim2.push_back(pRootSystem->getSubComponent(compName)->getPort(portName)->getLogDataVectorPtr()->at(i).at(dataId));
                            }
                        }
                        else
                        {
                            printErrorMessage("Could not load modelfile: " + modelfile);
                            return false;
                        }


                        for(size_t i=0; i<vTime.size(); ++i)
                        {
                            vRef.push_back(refData.interpolate(vTime[i], column));
                        }

                        //std::cout.rdbuf(cout_sbuf); // restore the original stream buffer

                        if(!compareVectors(vSim1, vRef, tolerance))
                        {
                            printColorMessage(Red, "Validation data test failed: " + toStdString(pRootSystem->getName()) + ":" + compName + ":" + portName + ":" + varname);
                            return false;
                        }

                        if(!compareVectors(vSim1, vSim2, tolerance))
                        {
                            printColorMessage(Red, "Consistency test failed (two consecutive simulations gave different results): " + toStdString(pRootSystem->getName()) + ":" + compName + ":" + portName + ":" + varname);
                            return false;
                        }

                        printColorMessage(Green, "Test successful: " + toStdString(pRootSystem->getName()) + ":" + compName + ":" + portName + ":" + varname);

                        pVariableNode = pVariableNode->next_sibling("variable");
                    }
                    pPortNode = pPortNode->next_sibling("port");
                }
                pComponentNode = pComponentNode->next_sibling("component");
            }
            pValidationNode = pValidationNode->next_sibling("validation");
        }
    }
    catch(std::exception &e)
    {
        printErrorMessage(string("Could not open or read file: ") + hvcFilePath + string(" ") + e.what());
        return false;
    }

    // Test was apparently succesfull or else we would not have reached this point
    return true;
}

