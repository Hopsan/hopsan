/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
