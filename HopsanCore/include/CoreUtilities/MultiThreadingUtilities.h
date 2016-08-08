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

#ifndef MULTITHREADINGUTILITIES_H
#define MULTITHREADINGUTILITIES_H

#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
#if __cplusplus > 199711L
#include <atomic>
#include <mutex>
#endif

#include "Component.h"
#include "ComponentSystem.h"

using namespace std;
using namespace hopsan;

size_t DLLIMPORTEXPORT determineActualNumberOfThreads(const size_t nDesiredThreads);


#if __cplusplus > 199711L

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


void simMaster(ComponentSystem *pSystem, vector<Component *> &sVector, vector<Component *> &cVector,
               vector<Component *> &qVector, vector<Node *> &nVector, vector<double *> &pSimTimes,
               double startTime, double timeStep, size_t numSimSteps, BarrierLock *pBarrier_S,
               BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N);

void simSlave(ComponentSystem *pSystem, vector<Component*> &sVector, vector<Component*> &cVector,
              vector<Component*> &qVector, vector<Node*> &nVector, double startTime,
              double timeStep, size_t numSimSteps, BarrierLock *pBarrier_S,
              BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N);

void simWholeSystems(vector<ComponentSystem *> systemPtrs, double stopTime);


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





void simPoolSlave(TaskPool *pTaskPoolC, TaskPool *pTaskPoolQ, std::atomic<double> *pTime, std::atomic<bool> *pStop);



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
        lastIdx=max(size_t(1), data.size())-1;
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









void simStealingMaster(ComponentSystem *pSystem,
                       vector<Component*> &sVector,
                       std::vector<ThreadSafeVector*> *cVectors,
                       std::vector<ThreadSafeVector*> *qVectors,
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
                       size_t maxSize);




void simStealingSlave(ComponentSystem *pSystem,
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

void simOneComponentOneStep(Component * pComp, double stopTime);


/////////////////////////////////////////////
// Parallel for loop algorithm using tasks //
/////////////////////////////////////////////

void simOneStep(std::vector<Component *> *pComponentPtrs, double stopTime);


#endif //C++11

#endif // MULTITHREADINGUTILITIES_H
