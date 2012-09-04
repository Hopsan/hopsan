#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>

#ifdef USETBB
#include "mutex.h"
#include "atomic.h"
#include "tick_count.h"
#include "task_group.h"
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
#ifdef WIN32
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
        // If user specifides a number of threads, attempt to use this number
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




