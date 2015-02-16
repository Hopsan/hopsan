#include "fmu2_hopsan.h"
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
    spCoreComponentSystem->setDesiredTimestep(0.001);
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
