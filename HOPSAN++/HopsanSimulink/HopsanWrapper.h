////#include <qtcore/qcoreapplication>
#include "../HopsanCore/HopsanCore.h"     //hopsan header file
#include "../HopsanCore/ComponentUtilities.h"
#include "../HopsanCore/HopsanEssentials.h"

//void testFucntion()
//{
//	double test;
//	test=5;
//	test = hopsan::limit(test, -0.1, 0.1);
//}


//
//#include <iostream>
//
using namespace hopsan;
using namespace std;

// static global variables
static ComponentSystem *spCoreComponentSystem;
static std::vector<string> sComponentNames;

//////////////////////////////////////
//// help functions - do not modify //
//////////////////////////////////////

void initSystem(double timestep)
{
	spCoreComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
    spCoreComponentSystem->setDesiredTimestep(timestep);
}

void addComponent(string name, string type)
{
    Component *pCoreComponent;
    pCoreComponent = HopsanEssentials::getInstance()->CreateComponent(type);
    spCoreComponentSystem->addComponent(pCoreComponent);
    pCoreComponent->setName(name);
    sComponentNames.push_back(name);
}

bool initComponents()
{
    if(spCoreComponentSystem->isSimulationOk())
    {
        for(size_t i=0; i<sComponentNames.size(); ++i)
        {
            spCoreComponentSystem->getComponent(sComponentNames.at(i))->initialize();
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
    spCoreComponentSystem->getComponent(compname)->setParameterValue(parname, value);
}

double getNodeData(string compname, string portname, int dataindex)
{
    return spCoreComponentSystem->getComponent(compname)->getPort(portname)->getDataVectorPtr()->back().at(dataindex);
}

bool simulateOneTimestep(double time)
{
    if(spCoreComponentSystem->isSimulationOk())
    {
        double timestep = spCoreComponentSystem->getDesiredTimeStep();
        spCoreComponentSystem->simulate(time, time+timestep);
        return true;
    }
    return false;
}
