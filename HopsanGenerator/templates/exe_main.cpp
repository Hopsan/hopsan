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
//! @file   exe_main.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2019-06-03
//!
//! @brief Contains main function for executable models
//!
//$Id$

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "HopsanCore.h"
#include "HopsanEssentials.h"
#include "ComponentSystem.h"

#include "exe_utilities.h"
#include "model.hpp"

using namespace hopsan;

static hopsan::ComponentSystem *spCoreComponentSystem = 0;
hopsan::HopsanEssentials gHopsanCore;

int main(int argc, char *argv[])
{
    bool simulationMode = false;
    Options options;

    //Instantiate model
    spCoreComponentSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), options.startT, options.stopT);
    if(!spCoreComponentSystem) {
        std::cout << "Failed to instantiate model!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return 1;
    }
    const HString modelName = spCoreComponentSystem->getName();
    spCoreComponentSystem->addSearchPath(modelName+"-resources");

    //Parse arguments
    std::vector<std::string> arguments(argv+1, argv + argc);
    for(std::string &arg : arguments) {
        if(arg.find("=") != std::string::npos) {
            std::string name = arg.substr(0, arg.find("="));
            std::string value = arg.substr(arg.find("=")+1, arg.size()-1);
            if("parameterfile" == name) {
                importParameterValuesFromCSV(value, spCoreComponentSystem);
            }
            else if("configfile" == name) {
                readConfigFile(value, options);
            }
            else if(!options.set(name, value)){
                //Attempt to set parameter
                if(!setParameter(name, value, spCoreComponentSystem)) {
                    std::cout << "Error: Unknown parameter: " << name << "\n";
                    return 1;
                }
            }
        }
        else if("--help" == arg || "-h" == arg) {
            printHelpText(spCoreComponentSystem);
            return 0;
        }
        else if("--parameters" == arg || "-p" == arg) {
            printParameters(spCoreComponentSystem);
            return 0;
        }
        else if("--simulate" == arg || "-s" == arg) {
            simulationMode = true;
        }
        else {
            std::cout << "Error: Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if(!simulationMode) {
        std::cout << "This is a stand-alone executable Hopsan model. Try '";
        std::cout << spCoreComponentSystem->getName().c_str();
        std::cout << " --help' for more information.\n";
        return 0;
    }

    spCoreComponentSystem->setDesiredTimestep(options.stepT);
    spCoreComponentSystem->setNumLogSamples(options.nSamples);

    std::cout << "Checking model... ";
    if (spCoreComponentSystem->checkModelBeforeSimulation()) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return 1;
    }

    std::cout << "Initializing model... ";
    if(spCoreComponentSystem->initialize(options.startT, options.stopT)) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return 1;
    }

    std::cout << "Simulating model... " << std::flush;
    std::thread simThread = std::thread(&hopsan::ComponentSystem::simulate,
                                        spCoreComponentSystem,
                                        options.stopT);

    if(options.progress) {
        double *pTime = spCoreComponentSystem->getTimePtr();
        int lastProgress = 0;
        int lastLength = 0;
        while((*pTime) < options.stopT-options.stepT*0.5 && !spCoreComponentSystem->wasSimulationAborted()) {
    #ifdef _WIN32
            Sleep(100);
    #else
            usleep(100000);
    #endif
            int progress = 100.0*((*pTime)-options.startT)/(options.stopT-options.startT);
            if(progress > lastProgress) {
                std::string progressStr = std::to_string(progress)+"%";
                for(int i=0; i<lastLength; ++i) {
                    std::cout << "\b";
                }
                std::cout << progressStr << std::flush;
                lastProgress = progress;
                lastLength = progressStr.size();
            }
        }
        for(int i=0; i<lastLength; ++i) {
            std::cout << "\b";
        }
    }
    simThread.join();
    std::cout << "Finished!\n";

    std::cout << "Finalizing model... ";
    spCoreComponentSystem->finalize();
    std::cout << "Finished!\n";

    std::cout << "Saving results... ";
    saveResults(spCoreComponentSystem, "output.csv", options.results, options.descriptions, "");
    std::cout << "Finished!\n";

    if(options.transpose) {
        std::cout << "Transposing results... ";
        transposeCSVresults("output.csv");
        std::cout << "Finished!\n";
    }

    return 0;
 }
