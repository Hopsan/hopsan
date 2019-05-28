#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>

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
    //Default values
    double startT = 0;
    double stopT = 10;
    double stepT = 0.001;
    double nSamples = 2048;
    bool transpose = false;
    SaveResults results = Full;
    SaveDescriptions descriptions = NameAliasUnit;

    //Instantiate model
    spCoreComponentSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), startT, stopT);
    if(!spCoreComponentSystem) {
        std::cout << "Failed to instantiate model!\n";
        return 1;
    }

    //Parse arguments
    std::vector<std::string> arguments(argv+1, argv + argc);
    for(std::string &arg : arguments) {
        if(arg.find("=") != std::string::npos) {
            std::string name = arg.substr(0, arg.find("="));
            std::string value = arg.substr(arg.find("=")+1, arg.size()-1);
            if(name == "start") {
                startT = atof(value.c_str());
            }
            else if("stop" == name) {
                stopT = atof(value.c_str());
            }
            else if("step" == name) {
                stepT = atof(value.c_str());
            }
            else if("samples" == name) {
                nSamples = atof(value.c_str());
            }
            else if("transpose" == name) {
                transpose = (value == "true");
            }
            else if("results" == name) {
                if(value == "final") {
                    results = Final;
                }
            }
            else if("descriptions" == name) {
                if(value == "namesonly") {
                    descriptions = NameOnly;
                }
            }
            else if("parameterfile" == name) {
                importParameterValuesFromCSV(value, spCoreComponentSystem);
            }
            else {
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
        else {
            std::cout << "Error: Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    spCoreComponentSystem->setDesiredTimestep(stepT);
    spCoreComponentSystem->setNumLogSamples(nSamples);

    std::cout << "Checking model... ";
    if (spCoreComponentSystem->checkModelBeforeSimulation()) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        return 1;
    }

    std::cout << "Initializing model... ";
    if(spCoreComponentSystem->initialize(startT, stopT)) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        return 1;
    }

    std::cout << "Simulating model... ";
    spCoreComponentSystem->simulate(stopT);;
    std::cout << "Finished!\n";

    std::cout << "Finalizing model... ";
    spCoreComponentSystem->finalize();
    std::cout << "Finished!\n";

    std::cout << "Saving results... ";
    saveResults(spCoreComponentSystem, "output.csv", results, descriptions, "");
    std::cout << "Finished!\n";

    if(transpose) {
        std::cout << "Transposing results... ";
        transposeCSVresults("output.csv");
        std::cout << "Finished!\n";
    }

    return 0;
 }
