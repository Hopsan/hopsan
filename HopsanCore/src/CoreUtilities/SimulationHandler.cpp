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

//!
//! @file   SimulationHandler.cpp
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains the simulation handler help class
//!
//$Id$

#include "CoreUtilities/SimulationHandler.h"
#include "CoreUtilities/MultiThreadingUtilities.h"
#include "ComponentSystem.h"

#if defined(HOPSANCORE_USEMULTITHREADING)
#include <thread>
#endif

using namespace hopsan;
using namespace std;

bool SimulationHandler::initializeSystem(const double startT, const double stopT, ComponentSystem* pSystem)
{
    if (pSystem->checkModelBeforeSimulation())
    {
        return pSystem->initialize(startT, stopT);
    }
    return false;
}

bool SimulationHandler::initializeSystem(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector)
{
    //No multicore init support
    bool isOk = true;
    for (size_t i=0; i<rSystemVector.size(); ++i)
    {
        isOk = isOk && initializeSystem(startT, stopT, rSystemVector[i]);
        if (!isOk)
        {
            break;
        }
    }
    return isOk;
}

bool SimulationHandler::simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges, ParallelAlgorithmT algorithm)
{
    if (nDesiredThreads < 0)
    {
        pSystem->addInfoMessage("Using single-threaded algorithm.");
        pSystem->simulate(stopT);
    }
    else
    {
        pSystem->simulateMultiThreaded(startT, stopT, nDesiredThreads, noChanges, algorithm);
    }

    return !pSystem->wasSimulationAborted();
}

bool SimulationHandler::simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges, ParallelAlgorithmT algorithm)
{
    if (rSystemVector.size() > 1)
    {
        if (nDesiredThreads >= 0)
        {
            return simulateMultipleSystemsMultiThreaded(startT, stopT, nDesiredThreads, rSystemVector, noChanges);
        }
        else
        {
            return simulateMultipleSystems(stopT, rSystemVector);
        }
    }
    else if (rSystemVector.size() == 1)
    {
        return simulateSystem(startT, stopT, nDesiredThreads, rSystemVector[0], noChanges, algorithm);
    }

    return false;
}

bool SimulationHandler::startRealtimeSimulation(ComponentSystem *pSystem, double realtimeFactor)
{
    return pSystem->startRealtimeSimulation(realtimeFactor);
}

void SimulationHandler::stopRealtimeSimulation(ComponentSystem *pSystem)
{
    pSystem->stopSimulation();
}

void SimulationHandler::finalizeSystem(ComponentSystem* pSystem)
{
    pSystem->finalize();
}

void SimulationHandler::finalizeSystem(std::vector<ComponentSystem*> &rSystemVector)
{
    //No multicore finalize
    for (size_t i=0; i<rSystemVector.size(); ++i)
    {
        finalizeSystem(rSystemVector[i]);
    }
}

//! @brief Distributes component system pointers evenly over one vector per thread, depending on their simulation time
//! @param[in] rSystemVector Vector to distribute
//! @param[in] nThreads Number of threads to distribute for
vector< vector<ComponentSystem *> > SimulationHandler::distributeSystems(const std::vector<ComponentSystem *> &rSystemVector, size_t nThreads)
{
    vector< vector<ComponentSystem *> > splitSystemVector;
    vector<double> timeVector;

    nThreads = min(nThreads, rSystemVector.size()); //Prevent adding for more threads then systems
    splitSystemVector.resize(nThreads);
    timeVector.resize(nThreads,0);
    size_t sysNum=0;
    while(true)         //! @todo Poor algorithm for distributing, will not give optimal results
    {
        for(size_t t=0; t<nThreads; ++t)
        {
            if(sysNum == rSystemVector.size())
                break;
            splitSystemVector[t].push_back(rSystemVector[sysNum]);
            timeVector[t] += rSystemVector[sysNum]->getMeasuredTime();
            ++sysNum;
        }
        if(sysNum == rSystemVector.size())
            break;
    }
    return splitSystemVector;
}

//! @brief Simulates several systems sequentially until given stop time
//! @param[in] stopT Stop time for all systems
//! @param[in] rSystemVector Vector of pointers to component systems to simulate
//! @returns true if successful else false if simulation was aborted for some reason
bool SimulationHandler::simulateMultipleSystems(const double stopT, const vector<ComponentSystem*> &rSystemVector)
{
    bool aborted = false;
    for(size_t i=0; i<rSystemVector.size(); ++i)
    {
        rSystemVector[i]->simulate(stopT);
        aborted = aborted && rSystemVector[i]->wasSimulationAborted(); //!< @todo this will give abort=true if one the systems fail, maybe we should abort entirely when one do
    }
    return !aborted;
}

//! @brief Simulates a vector of component systems in parallel, by assigning one or more system to each simulation thread
//! @param startT Start time for all systems
//! @param stopT Stop time for all systems
//! @param nDesiredThreads Desired number of threads (may change due to hardware limitations)
//! @param rSystemVector Vector of pointers to systems to simulate
bool SimulationHandler::simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const vector<ComponentSystem*> &rSystemVector, bool noChanges)
{
    HOPSAN_UNUSED(startT)
    HOPSAN_UNUSED(nDesiredThreads)
    HOPSAN_UNUSED(noChanges)

    vector<ComponentSystem*> tempSystemVector = rSystemVector;
#if defined(HOPSANCORE_USEMULTITHREADING)
    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);              //Calculate how many threads to actually use

    if(!noChanges)
    {
        mSplitSystemVector.clear();
        for(size_t i=0; i<tempSystemVector.size(); ++i)                     //Loop through the systems, set start time, log nodes and measure simulation time
        {
            tempSystemVector.at(i)->simulateAndMeasureTime(5);              //Measure time
            tempSystemVector.at(i)->initialize(startT, stopT);
        }
        sortSystemsByTotalMeasuredTime(tempSystemVector);                   //Sort systems by total measured time
        mSplitSystemVector = distributeSystems(tempSystemVector, nThreads); //Distribute systems evenly over split vectors
    }


    std::thread *tt = new std::thread[nThreads];  //Create simulation threads
    for (size_t t=0; t<nThreads; ++t)             //Execute simulation
    {
        tt[t] = std::thread(simWholeSystems,
                            mSplitSystemVector[t],
                            stopT);
    }
    for(size_t c=0; c<nThreads; ++c)              //Wait for all tasks to finish
    {
        tt[c].join();
    }
    delete[] tt;                                                 //Clean up

    bool aborted=false;
    for(size_t i=0; i<tempSystemVector.size(); ++i)
    {
        aborted = aborted && tempSystemVector[i]->wasSimulationAborted();
    }
    return !aborted;
#else
    // Use single core simulation if no multi-threading support
    return simulateMultipleSystems(stopT, rSystemVector);
#endif //C++11
}


//! @brief Sorts a vector of component system pointers by their required simulation time
//! @param [in out]systemVector Vector with system pointers to sort
#if __cplusplus >= 201103L
void SimulationHandler::sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector)
{
    size_t i, j;
    ComponentSystem *tempSystem;
    for(i = 1; i < rSystemVector.size(); ++i)
    {
        bool didSwap = false;
        for (j=0; j < (rSystemVector.size()-1); ++j)
        {
            if (rSystemVector[j+1]->getTotalMeasuredTime() > rSystemVector[j]->getTotalMeasuredTime())
            {
                tempSystem = rSystemVector[j];             //Swap elements
                rSystemVector[j] = rSystemVector[j+1];
                rSystemVector[j+1] = tempSystem;
                didSwap = true;               //Indicates that a swap occurred
            }
        }

        if(!didSwap)
        {
            break;
        }
    }
}
#else
void SimulationHandler::sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector)
{
    if(rSystemVector.size() > 0)
    {
        rSystemVector[0]->addErrorMessage("Sorting systems by measured time is not possible without the C++11 support.");
    }
    return;
}
#endif //C++11
