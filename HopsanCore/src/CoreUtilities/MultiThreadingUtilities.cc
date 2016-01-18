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

#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifndef USETBB
namespace tbb {
class mutex;
}
#endif

#include "CoreUtilities/MultiThreadingUtilities.h"



//! @brief Helper function that decides how many thread to use.
//! User specifies desired amount, but it is limited by how many cores the processor has.
//! @param [in] nDesiredThreads How many threads the user wants
//! @todo maybe this should be a core utility
size_t determineActualNumberOfThreads(const size_t nDesiredThreads)
{
    // Obtain number of processor cores from environment variable, or use user specified value if not zero
    size_t nThreads, nCores;
#ifdef _WIN32
    if(getenv("NUMBER_OF_PROCESSORS") != 0)
    {
        string temp = getenv("NUMBER_OF_PROCESSORS");
        nCores = atoi(temp.c_str());
    }
    else
    {
        nCores = 1;               //If non-Windows system, make sure there is at least one thread
    }
#else
    nCores = max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    if(nDesiredThreads != 0)
    {
        // If user specifies a number of threads, attempt to use this number
        // But limit number of threads to the number of system cores
        nThreads = min(nCores, nDesiredThreads);
    }
    else
    {
        //User specified nothing, (use auto), so use one thread per core
        nThreads = nCores;
    }
    return nThreads;
}






