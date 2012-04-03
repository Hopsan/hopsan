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

#include "HopsanEssentials.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>

#include <tclap/CmdLine.h>
#include "TicToc.hpp"

#include "ComponentUtilities/CSVParser.h"

// If we dont have the revision number then define UNKNOWN
// On real relase  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANCLISVNREVISION
#define HOPSANCLISVNREVISION "UNKNOWN"
#endif

#define HOPSANCLIVERSION "0.5.x_r" HOPSANCLISVNREVISION

#ifdef WIN32
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/components/defaultComponentLibrary.dll"
#else
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/components/libdefaultComponentLibrary.so"
#endif

using namespace std;
using namespace hopsan;


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


void printWaitingMessages(const bool printDebug=true)
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

void printTsInfo(const ComponentSystem* pSystem)
{
    cout << "Ts: " << pSystem->getDesiredTimeStep() << " InheritTs: " << pSystem->doesInheritTimestep();
}

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

void printSystemParams(ComponentSystem* pSystem)
{
    vector<string> names, values, units, descriptions, types;
    pSystem->getParameters(names,values,descriptions,units,types);

    for (size_t i=0; i<names.size(); ++i)
    {
        cout << " SysParam: " << names[i] << "=" << values[i];
    }
}

void printComponentHierarchy(ComponentSystem *pSystem, std::string prefix="",
                             const bool doPrintTsInfo=false,
                             const bool doPrintSystemParams=false)
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


//! @brief Compares a vector with a reference vector
//! @param vec Vector to compare
//! @param ref Reference vector
//! @param Tolereance (%) for acceptance
bool compareVectors(vector<double> vec, vector<double> ref, double tol)
{
    if(vec.size() != ref.size())
    {
        //! @todo Error message, size mismatch
        cout << "Size mismatch!";
        return false;
    }

    for(int i=0; i<vec.size(); ++i)
    {
        if(fabs(1-vec.at(i)/ref.at(i)) > tol && !(vec.at(i) < 1e-100 && ref.at(i) < 1e-10))
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
    refDataCurve = new CSVParser(success, modelName+".csv", ',', '\n', '"', 0);
    if(!success || !refDataCurve->checkData())
    {
        cout << "Unable to initialize CSV file: " << modelName+".csv";
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


    bool ok;
    for(size_t i=0; i<vTime.size(); ++i)
    {
        vRef.push_back(refDataCurve->interpolate(ok, vTime.at(i)));
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


int main(int argc, char *argv[])
{
    try {
        TCLAP::CmdLine cmd("HopsanCLI", ' ', HOPSANCLIVERSION);

        // Define a value argument and add it to the command line.
        TCLAP::ValueArg<std::string> hmfPathOption("f","hmf","The Hopsan model file to simulate",false,"","String containing file path", cmd);
        TCLAP::ValueArg<std::string> extLibPathsOption("e","ext","A file containing the external libs to load",false,"","String containing file path", cmd);
        TCLAP::ValueArg<std::string> saveNodesPathsOption("n", "savenodes", "A file containing lines with component name and portname to save node data from", false, "", "String containing file path", cmd);
        TCLAP::ValueArg<std::string> modelTestOption("t","test","Model test to perform",false,"","Model name", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        string hmfFilePath = hmfPathOption.getValue();
        string extFilePaths = extLibPathsOption.getValue();
        string testFilePath = modelTestOption.getValue();
        string saveNodeFilePath = saveNodesPathsOption.getValue();

        // Load default hopasn component lib
        HopsanEssentials::getInstance()->loadExternalComponentLib(DEFAULTCOMPONENTLIB);

        // Load external libs
        vector<string> extLibs;
        if (!extFilePaths.empty())
        {
            readExternalLibsFromTxtFile(extFilePaths,extLibs);
            for (size_t i=0; i<extLibs.size(); ++i)
            {
                HopsanEssentials::getInstance()->loadExternalComponentLib(extLibs[i]);
            }
        }

        if(!hmfFilePath.empty())
        {
            printWaitingMessages();

            double startTime=0, stopTime=2;
            ComponentSystem* pRootSystem = HopsanEssentials::getInstance()->loadHMFModel(hmfFilePath, startTime, stopTime);
            printWaitingMessages();

            cout << endl << "Component Hieararcy:" << endl << endl;
            printComponentHierarchy(pRootSystem, "", true, true);
            cout << endl;

            if (pRootSystem!=0)
            {
                TicToc initTimer("InitializeTime");
                bool initSuccess = pRootSystem->initialize(startTime, stopTime);
                initTimer.TocPrint();
                if (initSuccess)
                {
                    TicToc simuTimer("SimulationTime");
                    pRootSystem->simulate(startTime, stopTime);
                    simuTimer.TocPrint();
                }
                else
                {
                    cout << "Initialize failed, Simulation aborted!" << endl;
                }
            }

            //cout << endl << "Component Hieararcy:" << endl << endl;
            //printComponentHierarchy(pRootSystem, "", true);

            if (!saveNodeFilePath.empty())
            {
                cout << "Saving NodeData to file" << endl;
                vector<string> comps, ports;
                readNodesToSaveFromTxtFile(saveNodeFilePath, comps, ports);
                //saveNodeDataToFile(pRootSystem,"GainE","out","GainEout.txt");
                //saveNodeDataToFile(pRootSystem,"GainI","out","GainIout.txt");
                for (size_t i=0; i<comps.size(); ++i)
                {
                    string outfile = comps[i]+"_"+ports[i]+".txt";
                    saveNodeDataToFile(pRootSystem, comps[i], ports[i], outfile);
                }
            }
        }

        //Perform a unit test
        if(!testFilePath.empty())
        {
            performModelTest(testFilePath);
        }
        else
        {
            printWaitingMessages();
            cout << endl << "HopsanCLI Done!" << endl;
        }

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cout << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }


}
