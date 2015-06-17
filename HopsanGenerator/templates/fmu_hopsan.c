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

#include "fmu_hopsan.h"
#include "HopsanCore.h"
#include "HopsanEssentials.h"
#include <string>
#include "model.hpp"
#include <cassert>

static double fmu_time=0;
static hopsan::ComponentSystem *spCoreComponentSystem = 0;
static std::vector<std::string> sComponentNames;
hopsan::HopsanEssentials gHopsanCore;

double *dataPtrs[<<<nports>>>];

extern "C" {

void hopsan_instantiate()
{
    double startT;      //Dummy variable
    double stopT;       //Dummy variable
    spCoreComponentSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), startT, stopT);

    //Assert that model was successfully loaded
    assert(spCoreComponentSystem);

    //Initialize system
    spCoreComponentSystem->setDesiredTimestep(<<<timestep>>>);
    spCoreComponentSystem->disableLog();
    spCoreComponentSystem->initialize(0,10);

    <<<setdataptrs>>>
}

void hopsan_initialize()
{
    spCoreComponentSystem->initialize(0,10);
}


void hopsan_simulate(double stopTime)
{
    spCoreComponentSystem->simulate(stopTime);
}


double hopsan_get_real(int vr)
{
    return (*dataPtrs[vr]);
}

int hopsan_get_integer(int vr)
{
    //Write code here
}

int hopsan_get_boolean(int vr)
{
    //Write code here
}

const char* hopsan_get_string(int vr)
{
    //Write code here
}

void hopsan_set_real(int vr, double value)
{
    (*dataPtrs[vr]) = value;
}

void hopsan_set_integer(int vr, int value)
{
    //Write code here
}

void hopsan_set_boolean(int vr, int value)
{
    //Write code here
}

void hopsan_set_string(int vr, const char* value)
{
    //Write code here
}

}
