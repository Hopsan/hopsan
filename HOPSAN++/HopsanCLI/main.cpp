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

#ifndef BUILTINDEFAULTCOMPONENTLIB
#ifdef WIN32
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/defaultComponentLibrary.dll"
#else
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/libdefaultComponentLibrary.so"
#endif
#endif

using namespace std;
using namespace hopsan;

HopsanEssentials gHopsanCore;

int main(int argc, char *argv[])
{
    bool returnSuccess=false;
    try {
        TCLAP::CmdLine cmd("HopsanCLI", ' ', HOPSANCLIVERSION);

        // Define a value argument and add it to the command line.
        //TCLAP::ValueArg<std::string> saveNodesPathsOption("n", "savenodes", "A file containing lines with the ComponentName;PortName to save node data from", false, "", "FilePath string", cmd);
        TCLAP::ValueArg<std::string> resultCSVSort("", "resultCSVSort", "Store in columns or in rows: [rows, cols]", false, "rows", "string", cmd);
        TCLAP::ValueArg<std::string> resultTypeOption("", "resultType", "How the results should be exported, choose: [final_csv, full_csv]", false, "final_csv", "string", cmd);
        TCLAP::ValueArg<std::string> resultFileOption("", "resultFile", "where the results should be exported", false, "", "string", cmd);
        TCLAP::ValueArg<std::string> paramImportFile("", "paramImportFile", "CSV file with parameter values to import", false, "", "string", cmd);
        TCLAP::ValueArg<std::string> modelTestOption("t","validate","Model validation to perform",false,"",".hvc filePath", cmd);
        TCLAP::ValueArg<std::string> extLibPathsOption("e","ext","A file containing the external libs to load",false,"","FilePath string", cmd);
        TCLAP::ValueArg<std::string> hmfPathOption("m","hmf","The Hopsan model file to simulate",false,"","FilePath string", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

#ifndef BUILTINDEFAULTCOMPONENTLIB
        // Load default hopasn component lib
        gHopsanCore.loadExternalComponentLib(DEFAULTCOMPONENTLIB);
#endif

        // Load external libs
        vector<string> extLibs;
        if (extLibPathsOption.isSet())
        {
            readExternalLibsFromTxtFile(extLibPathsOption.getValue(), extLibs);
            for (size_t i=0; i<extLibs.size(); ++i)
            {
                gHopsanCore.loadExternalComponentLib(extLibs[i]);
            }
        }

        if(hmfPathOption.isSet())
        {
            printWaitingMessages();

            double startTime=0, stopTime=2;
            ComponentSystem* pRootSystem = gHopsanCore.loadHMFModel(hmfPathOption.getValue(), startTime, stopTime);
            printWaitingMessages();

            if (paramImportFile.isSet())
            {
                cout << "Setting parameter values from file: " << paramImportFile.getValue() << endl;
                importParameterValuesFromCSV(paramImportFile.getValue(), pRootSystem);
            }

            cout << endl << "Component Hieararcy:" << endl << endl;
            printComponentHierarchy(pRootSystem, "", true, true);
            cout << endl;

            if (pRootSystem)
            {
                //! @todo maybe use simulation handler object instead
                TicToc isoktimer("IsOkTime");
                bool initSuccess = pRootSystem->checkModelBeforeSimulation();
                isoktimer.TocPrint();
                TicToc initTimer("InitializeTime");
                initSuccess = initSuccess && pRootSystem->initialize(startTime, stopTime);
                initTimer.TocPrint();
                if (initSuccess)
                {
                    cout << "Simulating... Please Wait!" << endl;
                    TicToc simuTimer("SimulationTime");
                    pRootSystem->simulate(startTime, stopTime);
                    simuTimer.TocPrint();
                }
                else
                {
                    printWaitingMessages(false);
                    setColor(Red);
                    cout << "Initialize failed, Simulation aborted!" << endl;
                }

                pRootSystem->finalize();
            }

            //cout << endl << "Component Hieararcy:" << endl << endl;
            //printComponentHierarchy(pRootSystem, "", true);

//            if (!saveNodeFilePath.empty())
//            {
//                cout << "Saving NodeData to file" << endl;
//                vector<string> comps, ports;
//                readNodesToSaveFromTxtFile(saveNodeFilePath, comps, ports);
//                //saveNodeDataToFile(pRootSystem,"GainE","out","GainEout.txt");
//                //saveNodeDataToFile(pRootSystem,"GainI","out","GainIout.txt");
//                for (size_t i=0; i<comps.size(); ++i)
//                {
//                    string outfile = comps[i]+"_"+ports[i]+".txt";
//                    saveNodeDataToFile(pRootSystem, comps[i], ports[i], outfile);
//                }
//            }

            if (resultFileOption.isSet())
            {
                cout << "Saving results to file" << resultFileOption.getValue() << ", Using format: " << resultTypeOption.getValue() << endl;
                if (resultTypeOption.getValue() == "final_csv")
                {
                    saveFinalResults(pRootSystem, resultFileOption.getValue(), Final);
                }
                else if (resultTypeOption.getValue() == "full_csv")
                {
                    saveFinalResults(pRootSystem, resultFileOption.getValue(), Full);
                }
                else
                {
                    setColor(Red);
                    cout << "Unknown result type format: " << resultTypeOption.getValue() << endl;
                }

                // Should we transpose the result
                if (resultCSVSort.getValue() == "cols")
                {
                    cout << "Transposing CSV file" << endl;
                    transposeCSVresults(resultFileOption.getValue());
                }
                else if (resultCSVSort.getValue() != "rows")
                {
                    setColor(Red);
                    cout << "Unknown CSV sorting format: " << resultCSVSort.getValue() << endl;
                }
            }
        }

        // Perform a unit test
        if(modelTestOption.isSet())
        {
            returnSuccess = performModelTest(modelTestOption.getValue());
        }
        else
        {
            printWaitingMessages();
            returnSuccess=true;
            cout << endl << "HopsanCLI Done!" << endl;
        }

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cout << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }

    setColor(Reset); //Reset terminal color
    if (returnSuccess)
    {
        return 0;
    }
    // If not success return 1
    return 1;
}
