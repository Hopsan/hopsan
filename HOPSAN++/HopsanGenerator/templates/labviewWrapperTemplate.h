////#include <qtcore/qcoreapplication>
#include "HopsanCore.h"     //hopsan header file
#include "CoreUtilities/HmfLoader.h"
#include "ComponentUtilities.h"
#include "HopsanEssentials.h"

#include <sstream>
//
//#include <iostream>
//
//using namespace hopsan;
using namespace std;

// static global variables
static hopsan::HopsanEssentials gHopsanCore;
static hopsan::ComponentSystem *spCoreComponentSystem;
static std::vector<string> sComponentNames;

//////////////////////////////////////
//// help functions - do not modify //
//////////////////////////////////////

void createSystem(double timestep)
{
	spCoreComponentSystem = gHopsanCore.createComponentSystem();
    spCoreComponentSystem->setDesiredTimestep(timestep);
}

void loadModel(string path)
{
    double startT = 0.0;
    double stopT = 10.0;
	spCoreComponentSystem = gHopsanCore.loadHMFModel(path, startT, stopT);
}

void addComponent(string name, string type)
{
	hopsan::Component *pCoreComponent;
    pCoreComponent = gHopsanCore.createComponent(type);
    spCoreComponentSystem->addComponent(pCoreComponent);
    pCoreComponent->setName(name);
    sComponentNames.push_back(name);
}


bool initSystem()
{
	if(spCoreComponentSystem->checkModelBeforeSimulation())
	{
		return spCoreComponentSystem->initialize(0, 10);
	}
	return false;
}


bool initComponents()
{
    if(spCoreComponentSystem->checkModelBeforeSimulation())
    {
        for(size_t i=0; i<sComponentNames.size(); ++i)
        {
            spCoreComponentSystem->getSubComponentOrThisIfSysPort(sComponentNames.at(i))->initialize();
        }
        return true;
    }
    return false;
}


bool connect(string comp1, string port1, string comp2, string port2)
{
    return spCoreComponentSystem->connect(comp1, port1, comp2, port2);
}

void setParameter(string compname, string parname, double value)
{
	  std::stringstream ss;
	  ss << value;
    spCoreComponentSystem->getSubComponentOrThisIfSysPort(compname)->setParameterValue(parname, ss.str());
}

void writeNodeData(string compname, string portname, int dataindex, double data)
{
	spCoreComponentSystem->getSubComponentOrThisIfSysPort(compname)->getPort(portname)->writeNode(dataindex, data);
}

double readNodeData(string compname, string portname, int dataindex)
{
    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(compname)->getPort(portname)->readNode(dataindex);
}

bool simulateOneTimestep(double time)
{
    if(spCoreComponentSystem->checkModelBeforeSimulation())
    {
        double timestep = spCoreComponentSystem->getDesiredTimeStep();
        spCoreComponentSystem->simulate(time, time+timestep);
        return true;
    }
    return false;
}
