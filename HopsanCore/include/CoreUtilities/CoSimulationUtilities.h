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
