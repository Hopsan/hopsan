/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCLI/main.cpp
//! @author FluMeS
//! @date   2011-03-28
//!
//! @brief The HopsanCLI main file
//!
//$Id$

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

#include "ModelUtilities.h"
#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "TicToc.hpp"
#include "version_cli.h"
#include "CoreUtilities/SaveRestoreSimulationPoint.h"

#include "CliUtilities.h"
#include "ModelValidation.h"
#include "BuildUtilities.h"

// If debug extension has not already been defined then define it to prevent compilation error
#ifndef DEBUG_EXT
 #define DEBUG_EXT
#endif

#ifndef BUILTINDEFAULTCOMPONENTLIB
    #define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/" TO_STR(DLL_PREFIX) "defaultComponentLibrary" TO_STR(DEBUG_EXT) TO_STR(DLL_EXT)
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
        TCLAP::SwitchArg testInstanciateComponentsOption("", "testInstanciateComponents", "Create an instance of each registered component to look for errors.", cmd);
        TCLAP::SwitchArg endPauseOption("", "endPause", "Pauses the CLI at the end to let you see its output", cmd);
        TCLAP::SwitchArg printDebugOption("", "printDebug", "Show debug messages in the output", cmd);
        TCLAP::SwitchArg createHvcTestOption("", "createValidationData","Create a model validation data set based on the variables connected to scopes in the model given by option -m", cmd);
        TCLAP::SwitchArg prefixRootLevelName("", "prefixRootSystemName", "Prefix the root-level system name to exported results and parameters", cmd);

        TCLAP::ValueArg<std::string> buildCompLibOption("", "buildComponentLibrary", "Build the specified component library (point to the library xml)", false, "", "string", cmd);
        TCLAP::ValueArg<std::string> destinationOption("d","destination","Destination for resulting files",false,"","Path to directory", cmd);
        TCLAP::ValueArg<std::string> saveSimulationStateOption("", "saveSimState", "Export the simulation state to this file", false, "Path to file", "string", cmd);
        TCLAP::ValueArg<std::string> loadSimulationStateOption("", "loadSimState", "Load the simulation state to this file", false, "Path to file", "string", cmd);
        TCLAP::ValueArg<std::string> resultsCSVSortOption("", "resultsCSVSort", "Export results in columns or in rows: [rows, cols]", false, "rows", "string", cmd);
        TCLAP::ValueArg<std::string> resultsFinalCSVOption("", "resultsFinalCSV", "Export the results (only final values)", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> resultsFullCSVOption("", "resultsFullCSV", "Export the results (all logged data)", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> parameterExportOption("", "parameterExport", "CSV file with exported parameter values", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> parameterImportOption("", "parameterImport", "CSV file with parameter values to import", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> hvcTestOption("t","validate","Perform model validation based on HopsanValidationConfiguration",false,"","Path to .hvc file", cmd);
        TCLAP::ValueArg<std::string> nLogSamplesOption("l","numLogSamples","Set the number of log samples to store for the top-level system, (default: Use number in .hmf)",false,"","integer", cmd);
        TCLAP::ValueArg<std::string> simulateOption("s","simulate","Specify simulation time as: [hmf] or [start,ts,stop] or [ts,stop] or [stop]",false,"","Comma separated string", cmd);
        TCLAP::ValueArg<std::string> extLibsFileOption("","externalLibsFile","A text file containing the external libs to load",false,"","Path to file", cmd);
        TCLAP::MultiArg<std::string> extLibPathsOption("e","externalLib","Path to a .dll/.so/.dylib externalComponentLib. Can be given multiple times",false,"Path to file", cmd);
        TCLAP::ValueArg<std::string> hmfPathOption("m","hmf","The Hopsan model file to load",false,"","Path to file", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        std::string destinationPath = destinationOption.getValue();
        if (!destinationPath.empty())
        {
            if ( destinationPath[destinationPath.size()-1] != '/')
            {
                destinationPath.push_back('/');
            }
        }

        if (buildCompLibOption.isSet())
        {
            string output;
            cout << "Building component library: " << buildCompLibOption.getValue() << endl;
            bool rc = buildComponentLibrary(buildCompLibOption.getValue(), output);
            if (!rc)
            {
                printErrorMessage("Failed to build component library: "+buildCompLibOption.getValue());
            }
        }


#ifndef BUILTINDEFAULTCOMPONENTLIB
        // Load default hopasn component lib
        gHopsanCore.loadExternalComponentLib(DEFAULTCOMPONENTLIB);
#endif
        // Print initial core messages
        printWaitingMessages(printDebugOption.getValue());

        // Load external libs
        vector<string> externalComponentLibraries;
        // Check extLibs file options
        if (extLibsFileOption.isSet())
        {
            readExternalLibsFromTxtFile(extLibsFileOption.getValue(), externalComponentLibraries);
        }

        // Check the multiarg option
        for (size_t i=0; i<extLibPathsOption.getValue().size(); ++i)
        {
            externalComponentLibraries.push_back(extLibPathsOption.getValue()[i]);
        }

        // Load the actual external lib .dll/.so/.dylib files
        for (size_t i=0; i<externalComponentLibraries.size(); ++i)
        {
            bool rc = gHopsanCore.loadExternalComponentLib(externalComponentLibraries[i].c_str());
            printWaitingMessages(printDebugOption.getValue()); // Print after loading
            if (rc)
            {
                printColorMessage(Green, "Success loading External library: " + externalComponentLibraries[i]);
            }
            else
            {
                printErrorMessage("Failed to load External library: " + externalComponentLibraries[i]);
            }
        }

        if (testInstanciateComponentsOption.isSet())
        {
            cout <<  "Testing to instantiate each registered component. Any Error or Warning messages will be shown below:" << endl;
            //! @todo write as function
            vector<HString> types =  gHopsanCore.getRegisteredComponentTypes();
            size_t nErrors=0;
            for (size_t i=0; i<types.size(); ++i)
            {
                Component *pComp = gHopsanCore.createComponent(types[i].c_str());
                nErrors += gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages(); + gHopsanCore.getNumWarningMessages();
                printWaitingMessages(printDebugOption.getValue());
                gHopsanCore.removeComponent(pComp);
            }
            if (nErrors==0)
            {
                returnSuccess=true;
            }
        }

        if (hmfPathOption.isSet() && createHvcTestOption.getValue())
        {
            string model = hmfPathOption.getValue();
            string dst = destinationPath;
            string basePath, baseName, filename, ext;
            splitFilePath(model, basePath, filename);
            splitFileName(filename, baseName, ext);
            cout <<  "Creating HVC Validation Data Set from Model: " << model << endl << "Saving data to: " << dst+baseName << ".*" << endl;
            returnSuccess =  createModelTestDataSet(model, dst+baseName);
        }

        if(hmfPathOption.isSet() && !createHvcTestOption.getValue())
        {
            returnSuccess=false;
            printWaitingMessages(printDebugOption.getValue());

            if (hvcTestOption.isSet())
            {
                printWarningMessage("Do not specify a hmf file in combination with the -t (--validate) option. Model should be loaded from the .hvc file");
            }

            cout << "Loading Hopsan Model File: " << hmfPathOption.getValue() << endl;
            double startTime=0, stopTime=2;
            ComponentSystem* pRootSystem = gHopsanCore.loadHMFModelFile(hmfPathOption.getValue().c_str(), startTime, stopTime);
            size_t nErrors = gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages();
            printWaitingMessages(printDebugOption.getValue());
            if (nErrors < 1)
            {
                if (parameterImportOption.isSet())
                {
                    cout << "Importing parameter values from file: " << parameterImportOption.getValue() << endl;
                    importParameterValuesFromCSV(parameterImportOption.getValue(), pRootSystem);
                }

                if (parameterExportOption.isSet())
                {
                    cout << "Exporting parameter values to file: " << destinationPath+parameterExportOption.getValue() << endl;
                    string prefix;
                    if (prefixRootLevelName.getValue())
                    {
                        prefix = pRootSystem->getName().c_str()+string("$");
                    }
                    exportParameterValuesToCSV(destinationPath+parameterExportOption.getValue(), pRootSystem, prefix);
                }

                cout << endl << "Model Hierarchy:" << endl;
                printComponentHierarchy(pRootSystem, "", true, true);
                cout << endl;

                if (pRootSystem && simulateOption.isSet())
                {
                    bool doSimulate=true;

                    // Abort if root model is empty (probably load hmf failed somehow)
                    if (pRootSystem->isEmpty())
                    {
                        printErrorMessage("The root system seems to be empty, aborting!");
                        doSimulate = false;
                    }

                    double stepTime = pRootSystem->getTimestep();
                    // Parse simulation options, and replace hmf values if needed
                    vector<string> simTime;
                    if (simulateOption.getValue() != "hmf")
                    {
                        splitStringOnDelimiter(simulateOption.getValue(),',',simTime);
                        if (simTime.size() == 3)
                        {
                            startTime = atof(simTime[0].c_str());
                            stepTime = atof(simTime[1].c_str());
                            stopTime = atof(simTime[2].c_str());
                        }
                        else if (simTime.size() == 2)
                        {
                            stepTime = atof(simTime[0].c_str());
                            stopTime = atof(simTime[1].c_str());
                        }
                        else if (simTime.size() == 1)
                        {
                            stopTime = atof(simTime[0].c_str());
                        }
                    }
                    pRootSystem->setDesiredTimestep(stepTime);

                    if (nLogSamplesOption.isSet())
                    {
                        size_t nSamp = atoi(nLogSamplesOption.getValue().c_str());
                        cout << "Setting nLogSamples to: " << nSamp << endl;
                        pRootSystem->setNumLogSamples(nSamp);
                    }

                    // Apply loaded simulation states
                    if (loadSimulationStateOption.isSet())
                    {
                        double timeOffset;
                        restoreSimulationPoint(loadSimulationStateOption.getValue().c_str(), pRootSystem, timeOffset);
                        startTime+=timeOffset;
                        stopTime+=timeOffset;
                        pRootSystem->setKeepValuesAsStartValues(true);
                    }

                    //! @todo maybe use simulation handler object instead
                    TicToc isoktimer("IsOkTime");
                    doSimulate = doSimulate && pRootSystem->checkModelBeforeSimulation();
                    isoktimer.TocPrint();
                    if (doSimulate)
                    {
                        TicToc initTimer("InitializeTime");
                        doSimulate = doSimulate && pRootSystem->initialize(startTime, stopTime);
                        initTimer.TocPrint();
                    }
                    else
                    {
                        printWaitingMessages(printDebugOption.getValue());
                        printErrorMessage("Initialize failed, Simulation aborted!");
                    }

                    if (doSimulate)
                    {
                        cout << "Simulating: " << startTime << " to " << stopTime << " with Ts: " << stepTime << "     Please Wait!" << endl;
                        TicToc simuTimer("SimulationTime");
                        pRootSystem->simulate(stopTime);
                        simuTimer.TocPrint();
                    }
                    if (pRootSystem->wasSimulationAborted())
                    {
                        printErrorMessage("Simulation was aborted!");
                    }
                    else
                    {
                        returnSuccess = true;
                    }

                    pRootSystem->finalize();
                }

                printWaitingMessages(printDebugOption.getValue());

                // Check in what formats to export
                if (resultsFinalCSVOption.isSet())
                {
                    cout << "Saving Final results to file: " << destinationPath+resultsFinalCSVOption.getValue() << endl;
                    string prefix;
                    if (prefixRootLevelName.getValue())
                    {
                        prefix = pRootSystem->getName().c_str()+string("$");
                    }
                    saveResults(pRootSystem, destinationPath+resultsFinalCSVOption.getValue(), Final, prefix);
                    // Should we transpose the result
                    if (resultsCSVSortOption.getValue() == "cols")
                    {
                        cout << "Transposing CSV file" << endl;
                        transposeCSVresults(destinationPath+resultsFinalCSVOption.getValue());
                    }
                    else if (resultsCSVSortOption.getValue() != "rows")
                    {
                        printErrorMessage("Unknown CSV sorting format: " + resultsCSVSortOption.getValue());
                    }
                }

                if (resultsFullCSVOption.isSet())
                {
                    cout << "Saving Full results to file: " << destinationPath+resultsFullCSVOption.getValue() << endl;
                    string prefix;
                    if (prefixRootLevelName.getValue())
                    {
                        prefix = pRootSystem->getName().c_str()+string("$");
                    }
                    saveResults(pRootSystem, destinationPath+resultsFullCSVOption.getValue(), Full, prefix);
                    // Should we transpose the result
                    if (resultsCSVSortOption.getValue() == "cols")
                    {
                        cout << "Transposing CSV file" << endl;
                        transposeCSVresults(destinationPath+resultsFullCSVOption.getValue());
                    }
                    else if (resultsCSVSortOption.getValue() != "rows")
                    {
                        printErrorMessage("Unknown CSV sorting format: " + resultsCSVSortOption.getValue());
                    }
                }

                // Save simulation state
                if (saveSimulationStateOption.isSet())
                {
                    saveSimulationPoint(saveSimulationStateOption.getValue().c_str(), pRootSystem);
                }

                // Now remove the rootsystem
                delete pRootSystem;
                pRootSystem = 0;
            }
            else
            {
                printErrorMessage("There were errors while loading the model");
            }
        }

        // Perform a unit test
        if(hvcTestOption.isSet())
        {
            returnSuccess = performModelTest(hvcTestOption.getValue());
        }
        else
        {
            printWaitingMessages(printDebugOption.getValue());
            cout << endl << "HopsanCLI Done!" << endl;
        }

        setTerminalColor(Reset); //Reset terminal color
        if (endPauseOption.getValue())
        {
            cout << "Press ENTER to continue ...";
            cin.get();
        }

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cout << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }

    if (returnSuccess)
    {
        return 0;
    }
    // If not success return 1
    return 1;
}
