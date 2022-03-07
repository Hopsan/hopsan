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

#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#endif

#if __cplusplus >= 201103L
#include <mutex>
#include <chrono>
#include <ctime>
#include <thread>
#endif

#include "CoreUtilities/MultiThreadingUtilities.h"
#include "ComponentSystem.h"

namespace hopsan {

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
        std::string temp = getenv("NUMBER_OF_PROCESSORS");
        nCores = atoi(temp.c_str());
    }
    else
    {
        nCores = 1;               //If non-Windows system, make sure there is at least one thread
    }
#else
    nCores = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    if(nDesiredThreads != 0)
    {
        // If user specifies a number of threads, attempt to use this number
        // But limit number of threads to the number of system cores
        nThreads = std::min(nCores, nDesiredThreads);
    }
    else
    {
        //User specified nothing, (use auto), so use one thread per core
        nThreads = nCores;
    }
    return nThreads;
}


#if defined(HOPSANCORE_USEMULTITHREADING)

//! @brief Constructor for slave simulation thread function.
//! @param pSystem Pointer to top level component system
//! @param sVector Vector with signal components executed from this thread
//! @param cVector Vector with C-type components executed from this thread
//! @param qVector Vector with Q-type components executed from this thread
//! @param nVector Vector with nodes which is logged from this thread
//! @param startTime Start time of simulation
//! @param timeStep Step time of simulation
//! @param numSimSteps Number of simulation steps to run
//! @param *pBarrier_S Pointer to barrier before signal components
//! @param *pBarrier_C Pointer to barrier before C-type components
//! @param *pBarrier_Q Pointer to barrier before Q-type components
//! @param *pBarrier_N Pointer to barrier before node logging
void simSlave(ComponentSystem *pSystem,
              std::vector<Component*> &sVector,
              std::vector<Component*> &cVector,
              std::vector<Component*> &qVector,
              std::vector<Node*> &nVector,
              double startTime,
              double timeStep,
              size_t numSimSteps,
              BarrierLock *pBarrier_S,
              BarrierLock *pBarrier_C,
              BarrierLock *pBarrier_Q,
              BarrierLock *pBarrier_N)
{
    (void)nVector;

    double time = startTime;

    for(size_t i=0; i<numSimSteps; ++i)
    {
        time += timeStep;

        //! Signal Components !//

        pBarrier_S->increment();
        while(pBarrier_S->isLocked()){}                         //Wait at S barrier
        if(pSystem->wasSimulationAborted()) break;

        for(size_t i=0; i<sVector.size(); ++i)
        {
            sVector[i]->simulate(time);
        }


        //! C Components !//

        pBarrier_C->increment();
        while(pBarrier_C->isLocked()){}                         //Wait at C barrier
        if(pSystem->wasSimulationAborted()) break;

        for(size_t i=0; i<cVector.size(); ++i)
        {
            cVector[i]->simulate(time);
        }


        //! Q Components !//

        pBarrier_Q->increment();
        while(pBarrier_Q->isLocked()){}                         //Wait at Q barrier
        if(pSystem->wasSimulationAborted()) break;

        for(size_t i=0; i<qVector.size(); ++i)
        {
            qVector[i]->simulate(time);
        }

        //! Log Nodes !//

        pBarrier_N->increment();
        while(pBarrier_N->isLocked()){}                         //Wait at N barrier
        if(pSystem->wasSimulationAborted()) break;
        //! @todo Temporary hack by Peter, after rewriting how node data and time is logged this no longer works, now master thread loags all nodes, need to come up with something smart
        //            for(size_t i=0; i<mVectorN.size(); ++i)
        //            {
        //                mVectorN[i]->logData(time);
        //            }

    }
}


//! @brief Constructor for master simulation thread function.
//! @param pSystem Pointer to the top level component system
//! @param sVector Vector with signal components executed from this thread
//! @param cVector Vector with C-type components executed from this thread
//! @param qVector Vector with Q-type components executed from this thread
//! @param nVector Vector with nodes which is logged from this thread
//! @param *pSimTimes Pointer to the simulation time variables in the component systems
//! @param startTime Start time of simulation
//! @param timeStep Step time of simulation
//! @param numSimSteps Number of steps to simulate
//! @param *pBarrier_S Pointer to barrier before signal components
//! @param *pBarrier_C Pointer to barrier before C-type components
//! @param *pBarrier_Q Pointer to barrier before Q-type components
//! @param *pBarrier_N Pointer to barrier before node logging
void simMaster(ComponentSystem *pSystem, std::vector<Component *> &sVector, std::vector<Component *> &cVector,
               std::vector<Component *> &qVector, std::vector<Node *> &nVector, std::vector<double *> &pSimTimes, double startTime, double timeStep,
               size_t numSimSteps, BarrierLock *pBarrier_S, BarrierLock *pBarrier_C,
               BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N)
{
    (void)nVector;

    double time = startTime;

    for(size_t s=0; s<numSimSteps; ++s)
    {
        time += timeStep;

        //! Signal Components !//
        bool stop=false;
        while(!pBarrier_S->allArrived())   //Wait for all other threads to arrive at signal barrier
        {
            if(pSystem->wasSimulationAborted())
            {
                stop=true;
                break;
            }

        }
        if(stop)
        {
            pBarrier_S->unlock();
            pBarrier_C->unlock();
            pBarrier_Q->unlock();
            pBarrier_N->unlock();
            break;
        }
        pBarrier_C->lock();                    //Lock next barrier (must be done before unlocking this one, to prevent deadlocks)
        pBarrier_S->unlock();                  //Unlock signal barrier

        for(size_t i=0; i<sVector.size(); ++i)
        {
            sVector[i]->simulate(time);
        }

        //! C Components !//
        stop=false;
        while(!pBarrier_C->allArrived())   //C barrier
        {
            if(pSystem->wasSimulationAborted())
            {
                stop=true;
                break;
            }
        }
        if(stop)
        {
            pBarrier_S->unlock();
            pBarrier_C->unlock();
            pBarrier_Q->unlock();
            pBarrier_N->unlock();
            break;
        }
        pBarrier_Q->lock();
        pBarrier_C->unlock();

        for(size_t i=0; i<cVector.size(); ++i)
        {
            cVector[i]->simulate(time);
        }

        //! Q Components !//
        stop=false;
        while(!pBarrier_Q->allArrived()) //Q barrier
        {
            if(pSystem->wasSimulationAborted())
            {
                stop=true;
                break;
            }
        }
        if(stop)
        {
            pBarrier_S->unlock();
            pBarrier_C->unlock();
            pBarrier_Q->unlock();
            pBarrier_N->unlock();
            break;
        }
        pBarrier_N->lock();
        pBarrier_Q->unlock();
        for(size_t i=0; i<qVector.size(); ++i)
        {
            qVector[i]->simulate(time);
        }

        for(size_t i=0; i<pSimTimes.size(); ++i)
            *pSimTimes[i] = time;     //Update time in component system, so that progress bar can use it

        //! Log Nodes !//
        stop=false;
        while(!pBarrier_N->allArrived()) //N barrier
        {
            if(pSystem->wasSimulationAborted())
            {
                stop=true;
                break;
            }

        }
        if(stop)
        {
            pBarrier_S->unlock();
            pBarrier_C->unlock();
            pBarrier_Q->unlock();
            pBarrier_N->unlock();
            break;
        }
        pBarrier_S->lock();
        pBarrier_N->unlock();

        //! @todo Temporary hack by Peter, after rewriting how node data and time is logged this no longer works, now master thread loags all nodes, need to come up with something smart
        //            for(size_t i=0; i<mVectorN.size(); ++i)
        //            {
        //                mVectorN[i]->logData(time);
        //            }
        pSystem->logTimeAndNodes(s+1); //s+1 since at s=0 one simulation has been performed /Björn
    }
}


//! @brief Function for slave simulation threads using a task pool
void simPoolSlave(TaskPool *pTaskPoolC, TaskPool *pTaskPoolQ, std::atomic<double> *pTime, std::atomic<bool> *pStop)
{
    Component *pComp;
    while(!(*pStop))
    {
        //C-pool
        if(pTaskPoolC->isOpen())
        {
            pComp = pTaskPoolC->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolC->reportDone();
                pComp = pTaskPoolC->getComponent();
            }
            while(pTaskPoolC->isOpen()) {}
        }

        //Q-pool
        if(pTaskPoolQ->isOpen())
        {
            pComp = pTaskPoolQ->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolQ->reportDone();
                pComp = pTaskPoolQ->getComponent();
            }
            while(pTaskPoolQ->isOpen()) {}
        }
    }
}


//! @brief Function for master simulation thread, that is responsible for synchronizing the simulation
void simStealingMaster(ComponentSystem *pSystem,
                       std::vector<Component *> &sVector,
                       std::vector<ThreadSafeVector *> *cVectors,
                       std::vector<ThreadSafeVector *> *qVectors,
                       std::vector<double *> &pSimTimes,
                       double startTime,
                       double timeStep,
                       size_t numSimSteps,
                       size_t nThreads,
                       size_t threadID,
                       BarrierLock *pBarrier_S,
                       BarrierLock *pBarrier_C,
                       BarrierLock *pBarrier_Q,
                       BarrierLock *pBarrier_N,
                       size_t maxSize)
{
    ThreadSafeVector *pTemp;
    Component *pComp;

    double time = startTime;
    std::vector<ThreadSafeVector*> *pUnFinishedVectorsC = cVectors;
    std::vector<ThreadSafeVector*> *pUnFinishedVectorsQ = qVectors;
    ThreadSafeVector *pFinishedVectorC = new ThreadSafeVector(std::vector<Component*>(), maxSize);
    ThreadSafeVector *pFinishedVectorQ = new ThreadSafeVector(std::vector<Component*>(), maxSize);

    for(size_t s=0; s<numSimSteps; ++s)
    {
        if(pSystem->wasSimulationAborted()) break;

        time += timeStep;

        //! Signal Components !//

        while(!pBarrier_S->allArrived()) {}
        pBarrier_C->lock();
        pBarrier_S->unlock();

        //Simulate signal components
        for(size_t i=0; i<sVector.size(); ++i)
        {
            sVector[i]->simulate(time);
        }

        //! C Components !//

        while(!pBarrier_C->allArrived()) {}    //C barrier
        pBarrier_Q->lock();
        pBarrier_C->unlock();

        //SIMULATE C

        //Switch Q vectors
        pTemp = pUnFinishedVectorsQ->at(threadID);
        pUnFinishedVectorsQ->at(threadID) = pFinishedVectorQ;
        pFinishedVectorQ = pTemp;

        //Simulate own components
        pComp = pUnFinishedVectorsC->at(threadID)->tryAndTakeFirst();
        while(pComp)
        {
            pComp->simulate(time);
            pFinishedVectorC->insertFirst(pComp);
            pComp = pUnFinishedVectorsC->at(threadID)->tryAndTakeFirst();
        }

        //Steal components
        //if(int(time/timeStep/5+threadID)%nThreads == 0)
        {
            for(size_t i=0; i<nThreads-1; ++i)
            {
                int j = (threadID+1+i)%nThreads;
                pComp = pUnFinishedVectorsC->at(j)->tryAndTakeLast();
                if(pComp)
                {
                    pComp->simulate(time);
                    pFinishedVectorC->insertLast(pComp);
                    break;
                }
            }
        }

        //! Q Components !//

        while(!pBarrier_Q->allArrived()) {}    //Q barrier
        pBarrier_N->lock();
        pBarrier_Q->unlock();

        //SIMULATE Q

        //Switch C vectors
        pTemp = pUnFinishedVectorsC->at(threadID);
        pUnFinishedVectorsC->at(threadID) = pFinishedVectorC;
        pFinishedVectorC = pTemp;

        //Simulate own components
        pComp = pUnFinishedVectorsQ->at(threadID)->tryAndTakeFirst();
        while(pComp)
        {
            pComp->simulate(time);
            pFinishedVectorQ->insertFirst(pComp);
            pComp = pUnFinishedVectorsQ->at(threadID)->tryAndTakeFirst();
        }

        //Steal components
        //if(int(time/timeStep/5+mThreadID)%nThreads == 0)
        {
            for(size_t i=0; i<nThreads-1; ++i)
            {
                int j = (threadID+1+i)%nThreads;
                pComp = pUnFinishedVectorsQ->at(j)->tryAndTakeLast();
                if(pComp)
                {
                    pComp->simulate(time);
                    pFinishedVectorQ->insertLast(pComp);
                    break;
                }
            }
        }

        for(size_t i=0; i<pSimTimes.size(); ++i)
            *pSimTimes[i] = time;

        //! Log Nodes !//

        while(!pBarrier_N->allArrived()) {}    //N barrier
        pBarrier_S->lock();
        pBarrier_N->unlock();

        pSystem->logTimeAndNodes(s+1);
    }
}

void simStealingSlave(ComponentSystem *pSystem,
                      std::vector<ThreadSafeVector *> *cVectors,
                      std::vector<ThreadSafeVector *> *qVectors,
                      double startTime,
                      double timeStep,
                      size_t numSimSteps,
                      size_t nThreads,
                      size_t threadID,
                      BarrierLock *pBarrier_S,
                      BarrierLock *pBarrier_C,
                      BarrierLock *pBarrier_Q,
                      BarrierLock *pBarrier_N,
                      size_t maxSize)

{
    double time = startTime;;
    std::vector<ThreadSafeVector*> *pUnFinishedVectorsC = cVectors;
    std::vector<ThreadSafeVector*> *pUnFinishedVectorsQ = qVectors;
    //! @todo memory leak, never deleted, however, dont delete in destructor, that will cause double free corruption since this object is copied when new tasks are created
    ThreadSafeVector *pFinishedVectorC = new ThreadSafeVector(std::vector<Component*>(), maxSize);
    ThreadSafeVector *pFinishedVectorQ = new ThreadSafeVector(std::vector<Component*>(), maxSize);


    ThreadSafeVector *pTemp;
    Component *pComp;

    for(size_t i=0; i<numSimSteps; ++i)
    {
        if(pSystem->wasSimulationAborted()) break;

        time += timeStep;

        //! Signal Components !//

        pBarrier_S->increment();
        while(pBarrier_S->isLocked()){}                         //Wait at S barrier

        //! C Components !//

        pBarrier_C->increment();
        while(pBarrier_C->isLocked()){}                         //Wait at C barrier

        //C-COMPONENTS

        //Switch Q vectors
        pTemp = pUnFinishedVectorsQ->at(threadID);
        pUnFinishedVectorsQ->at(threadID) = pFinishedVectorQ;
        pFinishedVectorQ = pTemp;

        //Simulate own components
        pComp = pUnFinishedVectorsC->at(threadID)->tryAndTakeFirst();
        while(pComp)
        {
            pComp->simulate(time);
            pFinishedVectorC->insertFirst(pComp);
            pComp = pUnFinishedVectorsC->at(threadID)->tryAndTakeFirst();
        }

        //Steal components
        //if(int(time/timeStep/5+mThreadID)%mnThreads == 0)
        {
            for(size_t i=0; i<nThreads-1; ++i)
            {
                int j = (threadID+1+i)%nThreads;
                pComp = pUnFinishedVectorsC->at(j)->tryAndTakeLast();
                if(pComp)
                {
                    pComp->simulate(time);
                    pFinishedVectorC->insertLast(pComp);
                    break;
                }
            }
        }

        //! Q Components !//

        pBarrier_Q->increment();
        while(pBarrier_Q->isLocked()){}                         //Wait at Q barrier

        //Q-COMPONENTS

        //Switch C vectors
        pTemp = pUnFinishedVectorsC->at(threadID);
        pUnFinishedVectorsC->at(threadID) = pFinishedVectorC;
        pFinishedVectorC = pTemp;

        //Simulate own components
        pComp = pUnFinishedVectorsQ->at(threadID)->tryAndTakeFirst();
        while(pComp)
        {
            pComp->simulate(time);
            pFinishedVectorQ->insertFirst(pComp);
            pComp = pUnFinishedVectorsQ->at(threadID)->tryAndTakeFirst();
        }

        //Steal components
        //if(int(time/timeStep/5+mThreadID)%mnThreads == 0)
        {
            for(size_t i=0; i<nThreads-1; ++i)
            {
                int j = (threadID+1+i)%nThreads;
                pComp = pUnFinishedVectorsQ->at(j)->tryAndTakeLast();
                if(pComp)
                {
                    pComp->simulate(time);
                    pFinishedVectorQ->insertLast(pComp);
                    break;
                }
            }
        }

        //! Log Nodes !//

        pBarrier_N->increment();
        while(pBarrier_N->isLocked()){}                         //Wait at N barrier
    }
}

void simOneComponentOneStep(Component *pComp, double stopTime)
{
    pComp->simulate(stopTime);
}


void simOneStep(std::vector<Component *> *pComponentPtrs, double stopTime)
{
    size_t nComponents = pComponentPtrs->size();

    for (size_t i=0; i<nComponents; ++i)
    {
        pComponentPtrs->at(i)->simulate(stopTime);
    }
}


//! @brief Function for simulating whole systems multi-threaded
//! @param systemPtrs Vector with pointers to the systems to simulate
//! @param stopTime Stop time of simulation
void simWholeSystems(std::vector<ComponentSystem *> systemPtrs, double stopTime)
{
    for(size_t i=0; i<systemPtrs.size(); ++i)
    {
        systemPtrs[i]->simulate(stopTime);
    }
}

//! @brief Function for simulating whole systems multi-threaded
//! @param systemPtrs Vector with pointers to the systems to simulate
//! @param stopTime Stop time of simulation
void simWholeSystemInRealtime(double realTimeFactor, volatile bool *pStopSimulation, double *pTime, double timeStep, std::vector<Component*> signalComponentPtrs, std::vector<Component*> cComponentPtrs, std::vector<Component*> qComponentPtrs)
{
#if (__cplusplus >= 201103L)
    auto wallTime = std::chrono::steady_clock::now();

    while(!(*pStopSimulation)) {
        *pTime += timeStep; //mTime is updated here before the simulation,
        //mTime is the current time during the simulateOneTimestep

        //Signal components
        for(const auto &sComponent : signalComponentPtrs) {
            sComponent->simulate(*pTime);
        }

        //C components
        for(const auto &cComponent : cComponentPtrs) {
            cComponent->simulate(*pTime);
        }

        //Q components
        for(const auto &qComponent : qComponentPtrs) {
            qComponent->simulate(*pTime);
        }

        wallTime += std::chrono::microseconds(int(1000000*timeStep/realTimeFactor));
        std::this_thread::sleep_until(wallTime);
    }
#else
    return;
#endif
}

#endif //Multithreading

}


