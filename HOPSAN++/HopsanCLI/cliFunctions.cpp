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

#include "HopsanEssentials.h"
#include "ComponentUtilities/CSVParser.h"

using namespace std;
using namespace hopsan;

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
        if ((type != "debug") || printDebug)
        {
            cout << msg << endl;
        }
    }
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
//! @todo Make this support Linux as well
void setColor(unsigned int color)
{
#ifdef WIN32
    if (color >15 || color <=0)
    {
        cout <<"Error!" <<endl;
    }
    else
    {
        HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hcon,color);
    }
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
        //! @todo Error message, size mismatch
        cout << "Size mismatch!";
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
void performModelTest(string modelName)
{
    std::streambuf* cout_sbuf = std::cout.rdbuf(); // save original sbuf
    std::ofstream   fout("/dev/null");
    std::cout.rdbuf(fout.rdbuf()); // redirect 'cout' to a 'fout'

    string compName;
    string portName;
    int dataId;

    stringstream ss;
    ss << modelName << ".txt";
    ifstream ifs(ss.str().c_str());
    getline(ifs, compName);
    getline(ifs, portName);
    string temp;
    getline(ifs, temp);
    ifs.close();
    stringstream ss2(temp);
    ss2 >> dataId;

    cout << "Component name: " << compName;
    cout << "Port name: " << portName;
    cout << "Data name: " << dataId;

    vector<double> vRef;
    vector<double> vSim1;
    vector<double> vSim2;
    vector<double> vTime;


    //Load reference data curve
    CSVParser *refDataCurve;
    bool success=false;
    refDataCurve = new CSVParser(success, modelName+".csv", '\n', '"');
    if(!success || !refDataCurve->isInDataIncOrDec(0))
    {
        cout << "Unable to initialize CSV file: " << modelName+".csv " << refDataCurve->getErrorString();
        return;
    }

    double startTime=0, stopTime=1;
    ComponentSystem* pRootSystem = HopsanEssentials::getInstance()->loadHMFModel(modelName+".hmf", startTime, stopTime);
    printWaitingMessages();

    if (pRootSystem!=0)
    {
        //First simulation
        if (pRootSystem->initialize(startTime, stopTime))
        {
            pRootSystem->simulate(startTime, stopTime);
        }
        else
        {
            cout << "Initialize failed, Simulation aborted!" << endl;
            return;
        }

        for(size_t i=0; i<(*pRootSystem->getSubComponent(compName)->getPort(portName)->getTimeVectorPtr()).size(); ++i)
        {
            vTime.push_back((*pRootSystem->getSubComponent(compName)->getPort(portName)->getTimeVectorPtr()).at(i));
            vSim1.push_back(pRootSystem->getSubComponent(compName)->getPort(portName)->getDataVectorPtr()->at(i).at(dataId));
        }

        //Second simulation
        if (pRootSystem->initialize(startTime, stopTime))
        {
            pRootSystem->simulate(startTime, stopTime);
        }
        else
        {
            cout << "Initialize failed, Simulation aborted!" << endl;
            return;
        }

        for(size_t i=0; i<(*pRootSystem->getSubComponent(compName)->getPort(portName)->getTimeVectorPtr()).size(); ++i)
        {
            vSim2.push_back(pRootSystem->getSubComponent(compName)->getPort(portName)->getDataVectorPtr()->at(i).at(dataId));
        }
    }


    for(size_t i=0; i<vTime.size(); ++i)
    {
        vRef.push_back(refDataCurve->interpolate(vTime.at(i), 1));
    }

    std::cout.rdbuf(cout_sbuf); // restore the original stream buffer

    setColor(12);

    if(!compareVectors(vSim1, vRef, 0.01))
    {
        cout << "Test failed: " << pRootSystem->getName() << endl;
        setColor(15);
        return;
    }

    if(!compareVectors(vSim1, vSim2, 0.01))
    {
        cout << "Test failed (inconsistent result): " << pRootSystem->getName();
        setColor(15);
        return;
    }

    setColor(10);

    cout << "Test successful: " << pRootSystem->getName() << endl;

    setColor(15);

}

