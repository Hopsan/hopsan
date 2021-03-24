#include "hopsanc.h"
#include <iostream>
#include <string.h>
#include <vector>

#include "HopsanCore.h"
#include "HopsanEssentials.h"
#include "ComponentSystem.h"
#include "ComponentUtilities/num2string.hpp"

static hopsan::ComponentSystem *spCoreComponentSystem = nullptr;
static hopsan::HopsanEssentials gHopsanCore;

static double startTime, stopTime;

std::vector<hopsan::HString> msgVec;

//! @brief Puts specified message in message queue and prints it to cout
//! The queue is used by host environments that does not support printing couts, e.g. Matlab
//! @param [in] msg Message string
void printMessage(hopsan::HString msg) {
    msgVec.push_back(msg);
    std::cout << msg.c_str() << "\n";
}


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
                printMessage(msg);
            }
        }
        else {
            printMessage(msg);
        }
    }
}


//! @brief Reads a message from the message queue and removes it, unless queue is empty
//! Message will be truncated if buffer is too small
//! @param [in,out] buf Message buffer
//! @param [in] bufSize Buffer size
//! @returns Status (0 = success)
int getMessage(char* buf, size_t bufSize) {

    if(!msgVec.empty()) {
        if(bufSize < msgVec.at(0).size()) {
            msgVec.at(0) = msgVec.at(0).substr(0,bufSize);
        }
        strcpy(buf,msgVec.at(0).c_str());
        msgVec.erase(msgVec.begin());
        return 0;
    }
    return -1;
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
        printMessage("Failed to instantiate model!");
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }
    const hopsan::HString modelName = spCoreComponentSystem->getName();
    spCoreComponentSystem->addSearchPath(modelName+"-resources");
    printMessage("Loaded model: "+modelName);
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
        printMessage("Error: No model is loaded.");
        return -1;
    }

    //Parse variable string
    hopsan::HString varStr(variable);
    hopsan::HVector<hopsan::HString> splitSys = varStr.split('|');
    hopsan::HVector<hopsan::HString> splitVar = splitSys.last().split('.');
    splitSys.resize(splitSys.size()-1);

    //Find system
    hopsan::ComponentSystem *pSystem = spCoreComponentSystem;
    for(size_t i=0; i<splitSys.size(); ++i) {
        pSystem = pSystem->getSubComponentSystem(splitSys[i]);
        if(!pSystem) {
            printMessage("Error: Subsystem not found: "+splitSys[i]);
            return -1;
        }
    }

    //Check for alias if splitVar is of size one
    if(splitVar.size() == 1 && pSystem->getAliasHandler().hasAlias(splitVar[0])) {
        hopsan::HString compName, portName;
        int varId;
        pSystem->getAliasHandler().getVariableFromAlias(splitVar[0], compName, portName, varId);
        hopsan::Component *pComp = pSystem->getSubComponent(compName);
        hopsan::Port *pPort = pComp->getPort(portName);
        std::vector< std::vector<double> > *pLogData = pPort->getLogDataVectorPtr();
        for (size_t t=0; t<pSystem->getNumActuallyLoggedSamples(); ++t) {
            data[t] = (*pLogData)[t][size_t(varId)];
        }
        return 0;   //Found alias variable!
    }
    else if(splitVar.size() < 3) {
        printMessage("Error: Component name, port name and variable name must be specified.");
        return -1;
    }

    //Find component
    hopsan::Component *pComp = pSystem->getSubComponent(splitVar[0]);
    if(!pComp) {
        printMessage("Error: No such component: "+splitVar[0]);
        printMessage("Alternatives:");
        for(const hopsan::HString &name : pSystem->getSubComponentNames()) {
            printMessage("  "+name);
        }
        return -1;
    }

    //Find port
    hopsan::Port *pPort = pComp->getPort(splitVar[1]);
    if(!pPort) {
        printMessage("Error: No such port: "+splitVar[1]);
        printMessage("Alternatives:");
        for(const hopsan::HString &name : pComp->getPortNames()) {
            printMessage("  "+name);
        }
        return -1;
    }

    int varId = pPort->getNodeDataIdFromName(splitVar[2]);
    if(varId < 0) {
        printMessage("Error: No such variable: "+splitVar[2]);
        printMessage("Alternatives:");
        for(const auto &node : *pPort->getNodeDataDescriptions(0)) {
            printMessage("  "+node.name);
        }
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
        printMessage("Error: No model is loaded!");
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
        printMessage("Error: No model is loaded!");
        return -1;
    }
    printMessage("Checking model... ");
    if (spCoreComponentSystem->checkModelBeforeSimulation()) {
        printMessage("Success!");
    }
    else {
        printMessage("Failed!");
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }

    printMessage("Initializing... ");
    if(spCoreComponentSystem->initialize(startTime, stopTime)) {
        printMessage("Success!");
        printWaitingMessages(gHopsanCore, false, false);
    }
    else {
        printMessage("Failed!");
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }

    printMessage("Simulating... ");
    spCoreComponentSystem->simulate(stopTime);
    printMessage("Finished!");

    printMessage("Finalizing... ");
    spCoreComponentSystem->finalize();
    printMessage("Finished!");

    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}


//! @brief Provides time vector from last simulation
//! @param [in,out] data Buffer where data vector is stored (must be preallocated to match number of log samples)
//! @returns Status (0 = success)
int getTimeVector(double *data)
{
    if(!spCoreComponentSystem) {
        printMessage("Error: No model is loaded.");
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
        printMessage("Error: No model is loaded.");
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
        printMessage("Error: Parameter name not specified.");
        return -1;
    }

    //Find system
    hopsan::ComponentSystem *pSystem = spCoreComponentSystem;
    for(size_t i=0; i<sysVec.size(); ++i) {
        pSystem = pSystem->getSubComponentSystem(sysVec[i]);
        if(!pSystem) {
            printMessage("Error: Subsystem not found: "+sysVec[i]);
            return -1;
        }
    }

    if(compName.empty()) {   //Set system parameter
        if(pSystem->setParameterValue(parName, hopsan::HString(value))) {
            return 0;
        }
        else {
            printMessage("Error: Failed to set parameter value: "+parName);
            return -1;
        }
    }
    else if(nameVec.size() == 2 || nameVec.size() == 3) { //Set constant or input variable
        hopsan::Component *pComp = pSystem->getSubComponent(compName);
        if(!pComp) {
            printMessage("Error: No such component: "+compName);
            return -1;
        }
        if(pComp->setParameterValue(parName, hopsan::HString(value))) {
            return 0;
        }
        else {
            printMessage("Error: Failed to set parameter value: "+parName);
            return -1;
        }
    }

    printMessage("Error: Wrong number of arguments.");
    return -1;
}


//! @brief Specifies number of log samples for the simulation
//! @param [in] value Number of samples
//! @returns Status (0 = success)
int setNumberOfLogSamples(size_t value)
{
    if(!spCoreComponentSystem) {
        printMessage("Error: No model is loaded.");
        return -1;
    }
    printMessage("Setting samples to "+to_hstring(value));
    spCoreComponentSystem->setNumLogSamples(value);
    return 0;
}


//! @brief Returns number of logged samples from last simulation
//! @returns Number of samples
size_t getNumberOfLogSamples()
{
    if(!spCoreComponentSystem) {
        printMessage("Error: No model is loaded.");
        return 0;
    }
    return spCoreComponentSystem->getNumActuallyLoggedSamples();
}

int printWaitingMessages()
{
    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}
