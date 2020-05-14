#include "hopsanc.h"
#include <iostream>

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


int loadModel(const char* path) {
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


//! @brief Returns specified data vector as C array
//! @param [in] variable Variable name ("component.port.variable")
//! @param [out] size Size of returned data array
//! @returns Pointer to data array
double *getDataVector(const char* variable, int &size)
{
    if(!spCoreComponentSystem) {
        std::cout << "No model is loaded.\n";
        size = -1;
        return nullptr;
    }

    //Parse variable string
    hopsan::HString varStr(variable);
    hopsan::HVector<hopsan::HString>splitVar = varStr.split('.');
    if(splitVar.size() < 3) {
        std::cout << "Component name, port name and variable name must be specified.\n";
        size = -1;
        return nullptr;
    }

    //Find component
    hopsan::Component *pComp = spCoreComponentSystem->getSubComponent(splitVar[0]);
    if(!pComp) {
        std::cout << "No such component: " << splitVar[0].c_str() << "\n";
        size = -1;
        return nullptr;
    }

    //Find port
    hopsan::Port *pPort = pComp->getPort(splitVar[1]);
    if(!pPort) {
        std::cout << "No such port: " << splitVar[1].c_str() << "\n";
        size = -1;
        return nullptr;
    }

    int varId = pPort->getNodeDataIdFromName(splitVar[2]);
    if(varId < 0) {
        std::cout << "No such variable: " << splitVar[2].c_str() << "\n";
        size = -1;
        return nullptr;
    }

    size = int(spCoreComponentSystem->getNumActuallyLoggedSamples());
    double *data = new double[size];
    std::vector< std::vector<double> > *pLogData = pPort->getLogDataVectorPtr();
    for (size_t t=0; t<spCoreComponentSystem->getNumActuallyLoggedSamples(); ++t) {
        data[t] = (*pLogData)[t][size_t(varId)];
    }

    return data;
}

int loadLibrary(const char *path)
{
    if(!gHopsanCore.loadExternalComponentLib(path)) {
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    };
    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}

void setStartTime(double value)
{
    startTime = value;
}

void setTimeStep(double value)
{
    spCoreComponentSystem->setDesiredTimestep(value);
}

void setStopTime(double value)
{
    stopTime = value;
}

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

    std::cout << "Initializing model... ";
    if(spCoreComponentSystem->initialize(startTime, stopTime)) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        printWaitingMessages(gHopsanCore, false, false);
        return -1;
    }
    printWaitingMessages(gHopsanCore, false, false);

    std::cout << "Simulating... ";
    spCoreComponentSystem->simulate(stopTime);
    std::cout << "Finished!\n";

    printWaitingMessages(gHopsanCore, false, false);
    return 0;
}

double *getTimeVector(int &size)
{
    if(!spCoreComponentSystem) {
        size = -1;
        return nullptr;
    }
    size = int(spCoreComponentSystem->getNumActuallyLoggedSamples());
    return spCoreComponentSystem->getLogTimeVector()->data();
}



int setParameter(const char *name, const char *value)
{
    if(!spCoreComponentSystem) {
        std::cout << "No model is loaded.\n";
        return -1;
    }

    hopsan::HString nameStr(name);
    hopsan::HVector<hopsan::HString> nameVec = nameStr.split('.');

    if(nameVec.size() == 1) {
        if(spCoreComponentSystem->setParameterValue(nameVec[0], hopsan::HString(value))) {
            return 0;
        }
        else {
            std::cout << "Failed to set parameter value: " << nameVec[0].c_str() << "\n";
            return -1;
        }
    }
    else if(nameVec.size() == 2) {
        hopsan::Component *pComp = spCoreComponentSystem->getSubComponent(nameVec[0]);
        if(!pComp) {
            std::cout << "No such component: " << nameVec[0].c_str() << "\n";
            return -1;
        }
        if(pComp->setParameterValue(nameVec[1], hopsan::HString(value))) {
            return 0;
        }
        else {
            std::cout << "Failed to set parameter value: " << nameVec[1].c_str() << "\n";
            return -1;
        }
    }

    std::cout << "Wrong number of arguments.\n";
    return -1;
}

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
