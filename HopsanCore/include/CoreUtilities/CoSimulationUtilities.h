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

//$Id$

#ifndef COSIMULATIONUTILITIES_H
#define COSIMULATIONUTILITIES_H

#include "HopsanTypes.h"
#include <cstdlib>

//#define USEBOOST
#ifdef USEBOOST
#include "Dependencies/boost/boost/interprocess/shared_memory_object.hpp"
#include "Dependencies/boost/boost/interprocess/mapped_region.hpp"
#endif

//Some defines, to increase readability
#define MEMOBJ boost::interprocess::shared_memory_object
#define MEMREG boost::interprocess::mapped_region
#define BSTIPC boost::interprocess

namespace hopsan {

double *getDoubleSharedMemoryPointer(HString name);
bool *getBoolSharedMemoryPointer(HString name);
void removeSharedMemoryPointer(HString name);

}

#endif //COSIMULATIONUTILITIES_H
