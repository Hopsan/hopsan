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

