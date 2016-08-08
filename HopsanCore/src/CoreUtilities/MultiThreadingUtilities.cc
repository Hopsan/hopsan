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
#if __cplusplus > 199711L
#include <atomic>
#endif
#ifndef _WIN32
#include <unistd.h>
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


#if __cplusplus > 199711L

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
              vector<Component*> &sVector,
              vector<Component*> &cVector,
              vector<Component*> &qVector,
              vector<Node*> &nVector,
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
void simMaster(ComponentSystem *pSystem, vector<Component *> &sVector, vector<Component *> &cVector,
               vector<Component *> &qVector, vector<Node *> &nVector, vector<double *> &pSimTimes, double startTime, double timeStep,
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
                       vector<Component *> &sVector,
                       std::vector<ThreadSafeVector *> *cVectors,
                       std::vector<ThreadSafeVector *> *qVectors,
                       vector<double *> &pSimTimes,
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
void simWholeSystems(vector<ComponentSystem *> systemPtrs, double stopTime)
{
    for(size_t i=0; i<systemPtrs.size(); ++i)
    {
        systemPtrs[i]->simulate(stopTime);
    }
}

#endif //C++11


