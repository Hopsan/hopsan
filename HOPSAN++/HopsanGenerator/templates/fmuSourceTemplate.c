#include <iostream>
#include <sstream>
#include <assert.h>
#include "HopsanCore.h"
#include "Port.h"
#include "HopsanFMU.h"
#include "model.hpp"

static double fmu_time=0;
static hopsan::ComponentSystem *spCoreComponentSystem;
static std::vector<std::string> sComponentNames;
hopsan::HopsanEssentials gHopsanCore;

hopsan::Port *ports[<<<nports>>>];

void initializeHopsanWrapper(char* filename)
{
    double startT;      //Dummy variable
    double stopT;       //Dummy variable
    gHopsanCore.loadHMFModel("defaultComponentLibrary.dll");    //Only used for debugging, since components are not included in HopsanCore.dll during development
    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);

    assert(spCoreComponentSystem);
    spCoreComponentSystem->setDesiredTimestep(0.001);
    spCoreComponentSystem->disableLog();
    spCoreComponentSystem->initialize(0,10);

    fmu_time = 0;
}

void initializeHopsanWrapperFromBuiltInModel()
{
    spCoreComponentSystem = gHopsanCore.loadHMFModel(getModelString());

    assert(spCoreComponentSystem);
    spCoreComponentSystem->setDesiredTimestep(0.001);
    spCoreComponentSystem->disableLog();
    spCoreComponentSystem->initialize(0,10);

    fmu_time = 0;

    >>>assignportpointers>>>ports[<<<idx>>>] = spCoreComponentSystem->getSubComponent("<<<comp>>>")->getPort("<<<port>>>");
    <<<assignportpointers<<<
}

void simulateOneStep()
{
    double timestep = getTimeStep();
    spCoreComponentSystem->simulate(fmu_time, fmu_time+timestep);
    fmu_time = fmu_time+timestep;
}

double getTimeStep()
{
    return spCoreComponentSystem->getDesiredTimeStep();
}

double getFmuTime()
{
    return fmu_time;
}

double getVariable(size_t ref, size_t idx)
{
    return ports[ref]->readNode(idx);
}

void setVariable(size_t ref, size_t idx, double value)
{
    return ports[ref]->writeNode(idx, value);
}

void setParameter(char* name, double value)
{
    std::stringstream ss;
    ss << value;
    spCoreComponentSystem->setSystemParameter(name, ss.str(), "double");
    spCoreComponentSystem->initialize(0,10);
}
