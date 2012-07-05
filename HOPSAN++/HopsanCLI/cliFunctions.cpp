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

// ===== Help functions =====

//! @brief Helpfunction that splits a full path into basepath and filename
//! @note Assumes that dir separator is forward slash /
void splitFilePath(const std::string fullPath, std::string &rBasePath, std::string &rFileName)
{
    size_t pos = fullPath.rfind('/');
    // If not found try a windows backslash isntead
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

// ===== Print functions =====
//! @brief Prints all waiting messages
//! @param [in] printDebug Should debug messages also be printed
void printWaitingMessages(const bool printDebug)
{
    std::string msg,type,tag;
    cout << "Check messages: " << HopsanEssentials::getInstance()->checkMessage() << endl;
    while (HopsanEssentials::getInstance()->checkMessage() > 0)
    {
        HopsanEssentials::getInstance()->getMessage(msg,type,tag);
        //! @todo need yellow for warnings, or maybe orange
        if (type == "error")
        {
            setColor(Red);
        }
        else if (type == "debug")
        {
            setColor(Blue);
        }
        else
        {
            setColor(White);
        }

        if ((type != "debug") || printDebug)
        {
            cout << msg << endl;
        }
    }
    setColor(Reset);
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
        cout << " SysParam: " << pParams->at(i)->getName() << "=" << pParams->at(i)->getValue();
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
        cout << prefix << pSystem->getName() << " ";
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
        vector<string> names = pSystem->getSubComponentNames();
        for (size_t i=0; i<names.size(); ++i)
        {
            if ( pSystem->getSubComponent(names[i])->isComponentSystem() )
            {
                printComponentHierarchy(pSystem->getSubComponentSystem(names[i]), prefix, doPrintTsInfo, doPrintSystemParams);
            }
            else
            {
                cout << prefix << names[i] << endl;
            }
        }
    }
}

//! @brief Changes color on console output
//! @param color Color number (0-15)
//! @todo Need yellow color also
void setColor(const ColorsT color)
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
        c = "\e[31m";
        break;
    case Green:
        c = "\e[32m";
        break;
    case Blue:
        c = "\e[34m";
        break;
    case White:
    default:
        c = "\e[m";
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
            cout << "Error: Could not find portName: " << portName << endl;
            return; //Abort function
        }
        cout << "Error: Could not find compName: " << compName << endl;
    }
}

// ===== Load Functions =====
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
            cout << "line: " << line << endl;
            if (*line.begin() != '#')
            {
                rExtLibFileNames.push_back(line);
            }
        }
        file.close();
    }
    else
    {
        cout << "error, could not open file: " << filePath << endl;
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
        cout << "error, could not open file: " << filePath << endl;
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
        cout << "Error: compareVectors() Size mismatch!" << endl;
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
            setColor(Red);
            cout << hvcFilePath  << " Has wrong root tag name: " << string(pRootNode->name()) << endl;
            return false;
        }

        rapidxml::xml_node<> *pValidationNode = pRootNode->first_node("validation");
        if (!pValidationNode)
        {
            setColor(Red);
            cout << "Error: No validation node found in xml" << endl;
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
                setColor(Red);
                cout << "Error: No component node found in xml" << endl;
                return false;
            }
            while (pComponentNode != 0)
            {
                string compName = readStringAttribute(pComponentNode, "name", "_noname_");
                pPortNode = pComponentNode->first_node("port");
                if (!pPortNode)
                {
                    setColor(Red);
                    cout << "Error: No port node found in xml" << endl;
                    return false;
                }
                while (pPortNode != 0)
                {
                    string portName = readStringAttribute(pPortNode, "name", "_noname_");
                    string csvfile = readStringNodeValue(pPortNode->first_node("csvfile"), "");
                    pVariableNode = pPortNode->first_node("variable");
                    if (!pVariableNode)
                    {
                        setColor(Red);
                        cout << "Error: No variable node found in xml" << endl;
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
                            setColor(Red);
                            cout << "Unable to initialize CSV file: " << csvfile << " : " << refData.getErrorString() << endl;
                            return false;
                        }

                        double startTime=0, stopTime=1;
                        ComponentSystem* pRootSystem = HopsanEssentials::getInstance()->loadHMFModel(modelfile, startTime, stopTime);

                        if (pRootSystem!=0)
                        {
                            //! @todo maybe use simulation handler object
                            //First simulation
                            if (!pRootSystem->checkModelBeforeSimulation())
                            {
                                printWaitingMessages(false);
                                setColor(Red);
                                cout << "Initialize failed, Simulation aborted!" << endl;
                                return false;
                            }

                            if (pRootSystem->initialize(startTime, stopTime))
                            {
                                pRootSystem->simulate(startTime, stopTime);
                            }
                            else
                            {
                                printWaitingMessages(false);
                                setColor(Red);
                                cout << "Initialize failed, Simulation aborted!" << endl;
                                return false;
                            }
                            pRootSystem->finalize();

                            //copy the data
                            Component* pComp = pRootSystem->getSubComponent(compName);
                            if (!pComp)
                            {
                                setColor(Red);
                                cout << "Error: No such component name: " << compName << endl;
                                return false;
                            }
                            Port *pPort = pComp->getPort(portName);
                            if (!pPort)
                            {
                                setColor(Red);
                                cout << "Error: No such port name: " << portName << " in component: " << compName << endl;
                                return false;
                            }

                            //! @todo with better access function in core we could avoid copying data and work directly with the data stored
                            vTime = *pPort->getTimeVectorPtr();
                            int dataId = pPort->getNodeDataIdFromName(varname);
                            if (dataId < 0)
                            {
                                setColor(Red);
                                cout << "Error: No such varaiable name: " << varname << " in: " << pPort->getNodeType() << endl;
                                return false;
                            }

                            for(size_t i=0; i<vTime.size(); ++i)
                            {
                                vSim1.push_back(pRootSystem->getSubComponent(compName)->getPort(portName)->getDataVectorPtr()->at(i).at(dataId));
                            }

                            //Second simulation
                            if (pRootSystem->initialize(startTime, stopTime))
                            {
                                pRootSystem->simulate(startTime, stopTime);
                            }
                            else
                            {
                                printWaitingMessages(false);
                                setColor(Red);
                                cout << "Initialize failed, Simulation aborted!" << endl;
                                return false;
                            }
                            pRootSystem->finalize();

                            for(size_t i=0; i<vTime.size(); ++i)
                            {
                                vSim2.push_back(pRootSystem->getSubComponent(compName)->getPort(portName)->getDataVectorPtr()->at(i).at(dataId));
                            }
                        }
                        else
                        {
                            setColor(Red);
                            cout << "Error: Could not load modelfile: " << modelfile << endl;
                            return false;
                        }


                        for(size_t i=0; i<vTime.size(); ++i)
                        {
                            vRef.push_back(refData.interpolate(vTime[i], column));
                        }

                        //std::cout.rdbuf(cout_sbuf); // restore the original stream buffer

                        if(!compareVectors(vSim1, vRef, tolerance))
                        {
                            setColor(Red);
                            cout << "Validation data test failed: " << pRootSystem->getName() << ":" << compName << ":" << portName << ":" << varname << endl;
                            return false;
                        }

                        if(!compareVectors(vSim1, vSim2, tolerance))
                        {
                            setColor(Red);
                            cout << "Consistency test failed (two consecutive simulations gave different results): " << pRootSystem->getName() << ":" << compName << ":" << portName << ":" << varname << endl;
                            return false;
                        }

                        setColor(Green);
                        cout << "Test successful: " << pRootSystem->getName() << ":" << compName << ":" << portName << ":" << varname << endl;

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
        cout << "Error: Could not open or read file: " << hvcFilePath << " " << e.what() << endl;
        return false;
    }

    // Test was apparently succesfull or else we would not have reached this point
    return true;
}

