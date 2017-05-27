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

#include "CoreUtilities/CoSimulationUtilities.h"

#ifdef USEBOOST
#include "Dependencies/boost/boost/interprocess/shared_memory_object.hpp"
#include "Dependencies/boost/boost/interprocess/mapped_region.hpp"
#endif


#ifdef USEBOOST
//Maps are needed, because the shared memory objects and mapped regions must be kept alive as long as the memory pointer is used
std::map<HString, MEMOBJ*> sharedMemoryMap;
std::map<HString, MEMREG*> memoryRegionMap;
#endif

double *hopsan::getDoubleSharedMemoryPointer(HString name)
{
    (void)name;
#ifdef USEBOOST
    MEMOBJ *shdmem = new MEMOBJ(BSTIPC::open_or_create, name.c_str(), BSTIPC::read_write);
    sharedMemoryMap.insert(std::pair<HString, MEMOBJ*>(name, shdmem));
    shdmem->truncate(sizeof(double));
    MEMREG *region = new MEMREG(*shdmem, BSTIPC::read_write);
    memoryRegionMap.insert(std::pair<HString, MEMREG*>(name, region));
    double *mem_ptr = static_cast<double*>(region->get_address());
    return mem_ptr;
#else
    return 0;
#endif
}

bool *hopsan::getBoolSharedMemoryPointer(HString name)
{
    (void)name;
#ifdef USEBOOST
    MEMOBJ *shdmem = new MEMOBJ(BSTIPC::open_or_create, name.c_str(), BSTIPC::read_write);
    sharedMemoryMap.insert(std::pair<HString, MEMOBJ*>(name, shdmem));
    shdmem->truncate(sizeof(bool));
    MEMREG *region = new MEMREG(*shdmem, BSTIPC::read_write);
    memoryRegionMap.insert(std::pair<HString, MEMREG*>(name, region));
    bool *mem_ptr = static_cast<bool*>(region->get_address());
    return mem_ptr;
#else
    return 0;
#endif

}

void hopsan::removeSharedMemoryPointer(HString name)
{
    (void)name;
#ifdef USEBOOST
    sharedMemoryMap.erase(name);
    memoryRegionMap.erase(name);
    MEMOBJ::remove(name.c_str());
#endif
}

