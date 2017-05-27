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
    spCoreComponentSystem = gHopsanCore.loadHMFModel(path.c_str(), startT, stopT);
}

void addComponent(string name, string type)
{
	hopsan::Component *pCoreComponent;
    pCoreComponent = gHopsanCore.createComponent(type.c_str());
    spCoreComponentSystem->addComponent(pCoreComponent);
    pCoreComponent->setName(name.c_str());
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


bool connect(string comp1, string port1, string comp2, string port2)
{
    return spCoreComponentSystem->connect(comp1.c_str(), port1.c_str(), comp2.c_str(), port2.c_str());
}

void setParameter(string compname, string parname, double value)
{
	  std::stringstream ss;
	  ss << value;
    spCoreComponentSystem->getSubComponentOrThisIfSysPort(compname.c_str())->setParameterValue(parname.c_str(), ss.str().c_str());
}

void writeNodeData(string compname, string portname, int dataindex, double data)
{
    spCoreComponentSystem->getSubComponentOrThisIfSysPort(compname.c_str())->getPort(portname.c_str())->writeNode(dataindex, data);
}

double readNodeData(string compname, string portname, int dataindex)
{
    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(compname.c_str())->getPort(portname.c_str())->readNode(dataindex);
}

bool simulateOneTimestep(double time)
{
    if(spCoreComponentSystem->checkModelBeforeSimulation())
    {
        double timestep = spCoreComponentSystem->getDesiredTimeStep();
        spCoreComponentSystem->simulate(time+timestep);
        return true;
    }
    return false;
}
