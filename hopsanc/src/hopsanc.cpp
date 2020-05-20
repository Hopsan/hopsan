#include "hopsanc.h"
#include <iostream>
#include <string.h>

#include "HopsanCore.h"
#include "HopsanEssentials.h"
#include "ComponentSystem.h"

static hopsan::ComponentSystem *spCoreComponentSystem = nullptr;
static hopsan::HopsanEssentials gHopsanCore;

static double startTime, stopTime;

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
                std::cout << msg.c_str() << "\n";
            }
        }
        else {
            std::cout << msg.c_str() << "\n";
        }
    }
}



//! @brief Loads specified model file
//! @param [in] Full path to model file
//! @returns Status (0 = success)
int loadModel(const char* path) {
    if(spCoreComponentSystem) {
        delete spCoreComponentSystem;
    }
    spCoreComponentSystem = gHopsanCore.loadHMFModelFile(path, startTime, stopTime);
    if(!spCoreComponentSystem) {
        std::cout << "Failed to instantiate model!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }
    const hopsan::HString modelName = spCoreComponentSystem->getName();
    spCoreComponentSystem->addSearchPath(modelName+"-resources");
    std::cout << "Loaded model: " << modelName.c_str() << "\n";
    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}


//! @brief Provides specified data vector from last simulation
//! @param [in] variable Variable name ("component.port.variable")
//! @param [in,out] data Buffer where data vector is stored (must be preallocated to match number of log samples)
//! @returns Status (0 = success)
int getDataVector(const char* variable, double *data)
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded.\n";
        return -1;
    }

    //Parse variable string
    hopsan::HString varStr(variable);
    hopsan::HVector<hopsan::HString> splitSys = varStr.split('|');
    hopsan::HVector<hopsan::HString> splitVar = splitSys.last().split('.');
    splitSys.resize(splitSys.size()-1);
    if(splitVar.size() < 3) {
        std::cout << "Error: Component name, port name and variable name must be specified.\n";
        return -1;
    }

    //Find system
    hopsan::ComponentSystem *pSystem = spCoreComponentSystem;
    for(size_t i=0; i<splitSys.size(); ++i) {
        pSystem = pSystem->getSubComponentSystem(splitSys[i]);
        if(!pSystem) {
            std::cout << "Error: Subsystem not found: " << splitSys[i].c_str() << "\n";
            return -1;
        }
    }

    //Find component
    hopsan::Component *pComp = pSystem->getSubComponent(splitVar[0]);
    if(!pComp) {
        std::cout << "Error: No such component: " << splitVar[0].c_str() << "\n";
        return -1;
    }

    //Find port
    hopsan::Port *pPort = pComp->getPort(splitVar[1]);
    if(!pPort) {
        std::cout << "Error: No such port: " << splitVar[1].c_str() << "\n";
        return -1;
    }

    int varId = pPort->getNodeDataIdFromName(splitVar[2]);
    if(varId < 0) {
        std::cout << "Error: No such variable: " << splitVar[2].c_str() << "\n";
        return -1;
    }

    std::vector< std::vector<double> > *pLogData = pPort->getLogDataVectorPtr();
    for (size_t t=0; t<spCoreComponentSystem->getNumActuallyLoggedSamples(); ++t) {
        data[t] = (*pLogData)[t][size_t(varId)];
    }
    return 0;
}


//! @brief Loads specified component library
//! @param [in] Full path to binary file of component library
//! @returns Status (0 = success)
int loadLibrary(const char *path)
{
    if(!gHopsanCore.loadExternalComponentLib(path)) {
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    };
    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}


//! @brief Sets start time for simulation
//! @param [in] Start time
//! @returns Status (0 = success)
int setStartTime(double value)
{
    startTime = value;
    return 0;
}


//! @brief Sets time step for simulation
//! @param [in] Time step
//! @returns Status (0 = success)
int setTimeStep(double value)
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded!\n";
        return -1;
    }
    spCoreComponentSystem->setDesiredTimestep(value);
    return 0;
}


//! @brief Sets stop time for simulation
//! @param [in] Stop time
//! @returns Status (0 = success)
int setStopTime(double value)
{
    stopTime = value;
    return 0;
}


//! @brief Starts a simulation
//! @returns Status (0 = success)
int simulate()
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded!\n";
        return -1;
    }
    std::cout << "Checking model... ";
    if (spCoreComponentSystem->checkModelBeforeSimulation()) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }

    std::cout << "Initializing... ";
    if(spCoreComponentSystem->initialize(startTime, stopTime)) {
        std::cout << "Success!\n";
        printWaitingMessages(gHopsanCore, false, false);
    }
    else {
        std::cout << "Failed!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }

    std::cout << "Simulating... ";
    spCoreComponentSystem->simulate(stopTime);
    std::cout << "Finished!\n";

    std::cout << "Finalizing... ";
    spCoreComponentSystem->finalize();
    std::cout << "Finished!\n";

    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}


//! @brief Provides time vector from last simulation
//! @param [in,out] data Buffer where data vector is stored (must be preallocated to match number of log samples)
//! @returns Status (0 = success)
int getTimeVector(double *data)
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded.\n";
        return -1;
    }
    memcpy(data,spCoreComponentSystem->getLogTimeVector()->data(), spCoreComponentSystem->getNumActuallyLoggedSamples()*sizeof(double));
    return 0;
}


//! @brief Sets a parameter value
//! @param [in] name Name of parameter (with all qualifiers)
//! @param [in] value New value for parameter (will be converted from string to correct type)
//! @returns Status (0 = success)
int setParameter(const char *name, const char *value)
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded.\n";
        return -1;
    }

    //Parse arguments
    hopsan::HString nameStr(name);
    hopsan::HVector<hopsan::HString> sysVec = nameStr.split('|');
    hopsan::HVector<hopsan::HString> nameVec = sysVec.last().split('.');
    sysVec.resize(sysVec.size()-1);

    //Generate component name and parameter name
    hopsan::HString compName, parName;
    if(nameVec.size() == 1) {   //System parameter
        compName = "";
        parName = nameVec[0];
    }
    else if(nameVec.size() == 2) { //Constant
        compName = nameVec[0];
        parName = nameVec[1];
    }
    else if(nameVec.size() == 3) { //Input variable
        compName = nameVec[0];
        parName = nameVec[1]+"#"+nameVec[2];
    }
    else {
        std::cout << "Error: Parameter name not specified.\n";
        return -1;
    }

    //Find system
    hopsan::ComponentSystem *pSystem = spCoreComponentSystem;
    for(size_t i=0; i<sysVec.size(); ++i) {
        pSystem = pSystem->getSubComponentSystem(sysVec[i]);
        if(!pSystem) {
            std::cout << "Error: Subsystem not found: " << sysVec[i].c_str() << "\n";
            return -1;
        }
    }

    if(compName.empty()) {   //Set system parameter
        if(pSystem->setParameterValue(parName, hopsan::HString(value))) {
            return 0;
        }
        else {
            std::cout << "Error: Failed to set parameter value: " << parName.c_str() << "\n";
            return -1;
        }
    }
    else if(nameVec.size() == 2 || nameVec.size() == 3) { //Set constant or input variable
        hopsan::Component *pComp = pSystem->getSubComponent(compName);
        if(!pComp) {
            std::cout << "Error: No such component: " << compName.c_str() << "\n";
            return -1;
        }
        if(pComp->setParameterValue(parName, hopsan::HString(value))) {
            return 0;
        }
        else {
            std::cout << "Error: Failed to set parameter value: " << parName.c_str() << "\n";
            return -1;
        }
    }

    std::cout << "Error: Wrong number of arguments.\n";
    return -1;
}


//! @brief Specifies number of log samples for the simulation
//! @param [in] value Number of samples
//! @returns Status (0 = success)
int setLogSamples(unsigned long value)
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded.\n";
        return -1;
    }
    std::cout << "Setting samples to " << size_t(value) << "\n";
    spCoreComponentSystem->setNumLogSamples(value);
    return 0;
}


//! @brief Returns number of logged samples from last simulation
//! @returns Number of samples
size_t getLogSamples()
{
    if(!spCoreComponentSystem) {
        std::cout << "Error: No model is loaded.\n";
        return 0;
    }
    return spCoreComponentSystem->getNumActuallyLoggedSamples();
}
