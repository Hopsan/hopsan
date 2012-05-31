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
//! @file   main.cpp
//! @author FluMeS
//! @date   2011-03-28
//!
//! @brief Contains helpfunctions for CLI
//!
//$Id$

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

#include "cliFunctions.h"
#include "HopsanEssentials.h"
#include "TicToc.hpp"

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


int main(int argc, char *argv[])
{
    bool returnSuccess=true;
    try {
        TCLAP::CmdLine cmd("HopsanCLI", ' ', HOPSANCLIVERSION);

        // Define a value argument and add it to the command line.
        TCLAP::ValueArg<std::string> hmfPathOption("f","hmf","The Hopsan model file to simulate",false,"","FilePath string", cmd);
        TCLAP::ValueArg<std::string> extLibPathsOption("e","ext","A file containing the external libs to load",false,"","FilePath string", cmd);
        TCLAP::ValueArg<std::string> saveNodesPathsOption("n", "savenodes", "A file containing lines with the ComponentName;PortName to save node data from", false, "", "FilePath string", cmd);
        TCLAP::ValueArg<std::string> modelTestOption("t","test","Model test to perform",false,"","Model name", cmd);
        TCLAP::ValueArg<std::string> modelTestOptionXML("c","testc","Model test to perform",false,"","Model name", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        string hmfFilePath = hmfPathOption.getValue();
        string extFilePaths = extLibPathsOption.getValue();
        string testFilePath = modelTestOption.getValue();
        string saveNodeFilePath = saveNodesPathsOption.getValue();
        string testFilePathXML = modelTestOptionXML.getValue();


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
        else if (!testFilePathXML.empty())
        {
            returnSuccess = performModelTestXML(testFilePathXML);
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
        returnSuccess = false;
    }

    if (returnSuccess)
    {
        return 0;
    }
    // If not success return 1
    return 1;
}
