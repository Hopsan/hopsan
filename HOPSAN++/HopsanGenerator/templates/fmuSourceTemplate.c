#include <iostream>
#include <assert.h>
#include "HopsanCore.h"
#include "HopsanFMU.h"

static double fmu_time=0;
static hopsan::ComponentSystem *spCoreComponentSystem;
static std::vector<std::string> sComponentNames;
hopsan::HopsanEssentials gHopsanCore;

void initializeHopsanWrapper(char* filename)
{
    double startT;      //Dummy variable
    double stopT;       //Dummy variable
    gHopsanCore.loadExternalComponentLib("defaultComponentLibrary.dll");    //Only used for debugging, since components are not included in HopsanCore.dll during development
    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);

    assert(spCoreComponentSystem);
    spCoreComponentSystem->setDesiredTimestep(0.001);
    spCoreComponentSystem->initialize(0,10);

    fmu_time = 0;
}

void simulateOneStep()
{
    if(spCoreComponentSystem->checkModelBeforeSimulation())
    {
        double timestep = spCoreComponentSystem->getDesiredTimeStep();
        spCoreComponentSystem->simulate(fmu_time, fmu_time+timestep);
        fmu_time = fmu_time+timestep;

    }
    else
    {
        std::cout << "Simulation failed!";
    }
}

double getVariable(char* component, char* port, size_t idx)
{
    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->readNode(idx);
}

void setVariable(char* component, char* port, size_t idx, double value)
{
    assert(spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port) != 0);
    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->writeNode(idx, value);
}
