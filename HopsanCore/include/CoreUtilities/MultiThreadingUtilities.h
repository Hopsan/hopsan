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

#ifndef MULTITHREADINGUTILITIES_H
#define MULTITHREADINGUTILITIES_H

#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstddef>
#include <algorithm>
#include "win32dll.h"

#if (__cplusplus >= 201103L) && !defined(HOPSANCORE_NOMULTITHREADING)
#define HOPSANCORE_USEMULTITHREADING
#endif


namespace hopsan {

size_t HOPSANCORE_DLLAPI determineActualNumberOfThreads(const size_t nDesiredThreads);

}

#if defined(HOPSANCORE_USEMULTITHREADING)

#include <atomic>
#include <mutex>

namespace hopsan {

// Forward declaration
class Component;
class ComponentSystem;
class Node;

//! @brief Class for barrier locks in multi-threaded simulations.
class BarrierLock
{
public:
    //! @brief Constructor.
    //! @note Number of threads must be correct! Wrong value will result in either deadlocks or threads or non-synchronized threads.
    //! @param nThreads Number of threads to by synchronized.
    BarrierLock(size_t nThreads)
    {
        mnThreads=nThreads;
        mCounter = 0;
        mLock = true;
    }

    //! @brief Locks the barrier.
    inline void lock() { mCounter=0; mLock=true; }

    //! @brief Unlocks the barrier.
    inline void unlock() { mLock=false; }

    //! @brief Returns whether or not the barrier is locked.
    inline bool isLocked() { return mLock; }

    //! @brief Increments barrier counter by one.
    inline void increment() { ++mCounter; }

    //! @brief Returns whether or not all threads have incremented the barrier.
    inline bool allArrived() { return (mCounter == (mnThreads-1)); }      //One less due to master thread

private:
    int mnThreads;
    std::atomic<int> mCounter;
    std::atomic<bool> mLock;
};


HOPSANCORE_DLLAPI void simMaster(ComponentSystem *pSystem, std::vector<Component *> &sVector, std::vector<Component *> &cVector,
                                 std::vector<Component *> &qVector, std::vector<Node *> &nVector, std::vector<double *> &pSimTimes,
                                 double startTime, double timeStep, size_t numSimSteps, BarrierLock *pBarrier_S,
                                 BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N);

HOPSANCORE_DLLAPI void simSlave(ComponentSystem *pSystem, std::vector<Component*> &sVector, std::vector<Component*> &cVector,
                                std::vector<Component*> &qVector, std::vector<Node*> &nVector, double startTime,
                                double timeStep, size_t numSimSteps, BarrierLock *pBarrier_S,
                                BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N);

HOPSANCORE_DLLAPI void simWholeSystemInRealtime(double realTimeFactor, volatile bool *pStopSimulation, double *pTime, double timeStep, std::vector<Component *> signalComponentPtrs, std::vector<Component *> cComponentPtrs, std::vector<Component *> qComponentPtrs);

HOPSANCORE_DLLAPI void simWholeSystems(std::vector<ComponentSystem *> systemPtrs, double stopTime);


//////////////////////////////
// Task Pool Implementation //
//////////////////////////////


class TaskPool
{
public:
    TaskPool(std::vector<Component*> componentPtrs)
    {
        mComponentPtrs = componentPtrs;
        mSize = componentPtrs.size();
        mCurrentIdx.store(0);
        mnDone = 0;
        mOpen=false;
    }

    Component *getComponent()
    {
        size_t idx = std::atomic_fetch_add(&mCurrentIdx,size_t(1));
        if(idx >= mSize)
            return 0;
        else
            return mComponentPtrs[idx];
    }

    void reportDone()
    {
        atomic_fetch_add(&mnDone,size_t(1));
    }

    bool isReady()
    {
        return mnDone >= mSize;
    }

    void open()
    {
        mCurrentIdx.store(0);
        mnDone.store(0);
        mOpen.store(true);
    }

    void close()
    {
        std::atomic_store(&mOpen,false);
    }

    bool isOpen()
    {
        return mOpen;
    }

private:
    std::vector<Component*> mComponentPtrs;
    size_t mSize;
    std::atomic<size_t> mCurrentIdx;
    std::atomic<size_t> mnDone;
    std::atomic<bool> mOpen;
};





HOPSANCORE_DLLAPI void simPoolSlave(TaskPool *pTaskPoolC, TaskPool *pTaskPoolQ, std::atomic<double> *pTime, std::atomic<bool> *pStop);



/////////////////////////////
// Task-stealing algorithm //
/////////////////////////////


class ThreadSafeVector
{
public:
    ThreadSafeVector(std::vector<Component*> data, size_t maxSize)
    {
        mSize = maxSize;
        mVector.resize(maxSize);
        for(size_t i=0; i<data.size(); ++i)
        {
            mVector[i] = data[i];
        }
        firstIdx=0;
        lastIdx=std::max(size_t(1), data.size())-1;
        mpMutex = new std::mutex();
    }


    Component *tryAndTakeFirst()
    {
        Component *ret;
        mpMutex->lock();

        int idx = (firstIdx%mSize+mSize)%mSize;
        ret = mVector[idx];
        mVector[idx] = 0;

        if(firstIdx != lastIdx)
            ++firstIdx;

        mpMutex->unlock();
        return ret;
    }

    Component *tryAndTakeLast()
    {
        Component *ret = 0;
        if(!mpMutex->try_lock())
            return 0;

        int idx = (lastIdx%mSize+mSize)%mSize;
        ret = mVector[idx];
        mVector[idx] = 0;

        if(lastIdx != firstIdx)
            --lastIdx;

        mpMutex->unlock();
        return ret;
    }

    //! @note NOT THREAD-SAFE
    size_t size()
    {
        return mVector.size();
    }

    void insertFirst(Component* comp)
    {
        mpMutex->lock();
        int idx = (firstIdx%mSize+mSize)%mSize;
        if(firstIdx == lastIdx && mVector[idx] == 0)
        {
            mVector[idx] = comp;
        }
        else
        {
            --firstIdx;
            idx = (firstIdx%mSize+mSize)%mSize;
            mVector[idx] = comp;
        }
        mpMutex->unlock();
    }


    void insertLast(Component* comp)
    {
        mpMutex->lock();
        int idx = (firstIdx%mSize+mSize)%mSize;
        if(firstIdx == lastIdx && mVector[idx] == 0)
        {
            mVector[idx] = comp;
        }
        else
        {
            ++lastIdx;
            int idx = (lastIdx%mSize+mSize)%mSize;
            mVector[idx] = comp;
        }
        mpMutex->unlock();
    }

    //! @note NOT THREAD-SAFE
    Component* at(size_t idx)
    {
        return mVector[idx];
    }

private:
    int mSize;
    std::vector<Component*> mVector;
    int firstIdx, lastIdx;
    std::mutex *mpMutex;
};


HOPSANCORE_DLLAPI void simStealingMaster(ComponentSystem *pSystem,
                                         std::vector<Component*> &sVector,
                                         std::vector<ThreadSafeVector*> *cVectors,
                                         std::vector<ThreadSafeVector*> *qVectors,
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
                                         size_t maxSize);




HOPSANCORE_DLLAPI void simStealingSlave(ComponentSystem *pSystem,
                                        std::vector<ThreadSafeVector*> *cVectors,
                                        std::vector<ThreadSafeVector*> *qVectors,
                                        double startTime,
                                        double timeStep,
                                        size_t numSimSteps,
                                        size_t nThreads,
                                        size_t threadID,
                                        BarrierLock *pBarrier_S,
                                        BarrierLock *pBarrier_C,
                                        BarrierLock *pBarrier_Q,
                                        BarrierLock *pBarrier_N,
                                        size_t maxSize);


/////////////////////////////////////////////
// Parallel for loop algorithm using tasks //
/////////////////////////////////////////////

HOPSANCORE_DLLAPI void simOneComponentOneStep(Component * pComp, double stopTime);


/////////////////////////////////////////////
// Parallel for loop algorithm using tasks //
/////////////////////////////////////////////

HOPSANCORE_DLLAPI void simOneStep(std::vector<Component *> *pComponentPtrs, double stopTime);

}

#endif //C++11 and threading

#endif // MULTITHREADINGUTILITIES_H
