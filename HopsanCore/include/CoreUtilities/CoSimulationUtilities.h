#ifndef COSIMULATIONUTILITIES_H
#define COSIMULATIONUTILITIES_H

#include "HopsanTypes.h"
#include <cstdlib>

//#define USEBOOST
#ifdef USEBOOST
#include "Dependencies/boost/boost/interprocess/shared_memory_object.hpp"
#include "Dependencies/boost/boost/interprocess/mapped_region.hpp"
#endif

//Some defines, to inrease readability
#define MEMOBJ boost::interprocess::shared_memory_object
#define MEMREG boost::interprocess::mapped_region
#define BSTIPC boost::interprocess

namespace hopsan {

double *getDoubleSharedMemoryPointer(HString name);
bool *getBoolSharedMemoryPointer(HString name);
void removeSharedMemoryPointer(HString name);

}

#endif //COSIMULATIONUTILITIES_H
