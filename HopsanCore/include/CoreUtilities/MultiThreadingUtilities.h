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

#ifdef USETBB
#include "tbb/atomic.h"
#include "tbb/blocked_range.h"
#include "tbb/mutex.h"
#include "tbb/task_group.h"
#include "tbb/tick_count.h"
#endif

#include "Component.h"
#include "ComponentSystem.h"

using namespace std;
using namespace hopsan;

size_t DLLIMPORTEXPORT determineActualNumberOfThreads(const size_t nDesiredThreads);

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
#ifdef USETBB
    tbb::atomic<int> mCounter;
    tbb::atomic<bool> mLock;
#else
    int mCounter;
    bool mLock;
#endif
};



//! @brief Class for slave simulation threads, which must be synchronized from a master simulation thread
class taskSimOneComponent
{
public:
    taskSimOneComponent(Component* pComponent, double time)
    {
        mpComponent = pComponent;
        mTime = time;
    }

    void operator() ()
    {
        mpComponent->simulate(mTime);
    }
private:
    Component *mpComponent;
    double mTime;
};





//! @brief Class for slave simulation threads, which must be synchronized from a master simulation thread
class taskSimSlave
{
public:
    //! @brief Constructor for simulation thread class.
    //! @param sVector Vector with signal components executed from this thread
    //! @param cVector Vector with C-type components executed from this thread
    //! @param qVector Vector with Q-type components executed from this thread
    //! @param nVector Vector with nodes which is logged from this thread
    //! @param startTime Start time of simulation
    //! @param timeStep Step time of simulation
    //! @param stopTime Stop Time of simulation
    //! @param nThreads Number of threads used in simulation
    //! @param threadID Number of this thread
    //! @param *pBarrier_S Pointer to barrier before signal components
    //! @param *pBarrier_C Pointer to barrier before C-type components
    //! @param *pBarrier_Q Pointer to barrier before Q-type components
    //! @param *pBarrier_N Pointer to barrier before node logging
    taskSimSlave(ComponentSystem *pSystem, vector<Component*> sVector, vector<Component*> cVector, vector<Component*> qVector, vector<Node*> nVector,
                 double startTime, double timeStep, size_t numSimSteps, size_t nThreads, size_t threadID,
                 BarrierLock *pBarrier_S, BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N)
    {
        mpSystem = pSystem;
        mVectorS = sVector;
        mVectorC = cVector;
        mVectorQ = qVector;
        mVectorN = nVector;
        mTime = startTime;
        mNumSimSteps = numSimSteps;
        mTimeStep = timeStep;
        mnThreads = nThreads;
        mThreadID = threadID;
        mpBarrier_S = pBarrier_S;
        mpBarrier_C = pBarrier_C;
        mpBarrier_Q = pBarrier_Q;
        mpBarrier_N = pBarrier_N;
    }

    //! @brief Executable code for slave simulation thread
    void operator() ()
    {
        for(size_t i=0; i<mNumSimSteps; ++i)
        {
            if(mpSystem->wasSimulationAborted()) break;

            mTime += mTimeStep;

            //! Signal Components !//

            mpBarrier_S->increment();
            while(mpBarrier_S->isLocked()){}                         //Wait at S barrier

            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime);
            }


            //! C Components !//

            mpBarrier_C->increment();
            while(mpBarrier_C->isLocked()){}                         //Wait at C barrier

            for(size_t i=0; i<mVectorC.size(); ++i)
            {
                mVectorC[i]->simulate(mTime);
            }


            //! Q Components !//

            mpBarrier_Q->increment();
            while(mpBarrier_Q->isLocked()){}                         //Wait at Q barrier

            for(size_t i=0; i<mVectorQ.size(); ++i)
            {
                mVectorQ[i]->simulate(mTime);
            }

            //! Log Nodes !//

            mpBarrier_N->increment();
            while(mpBarrier_N->isLocked()){}                         //Wait at N barrier

            //! @todo Temporary hack by Peter, after rewriting how node data and time is logged this no longer works, now master thread loags all nodes, need to come up with something smart
//            for(size_t i=0; i<mVectorN.size(); ++i)
//            {
//                mVectorN[i]->logData(mTime);
//            }

        }
    }
private:
    ComponentSystem *mpSystem;
    vector<Component*> mVectorS;
    vector<Component*> mVectorC;
    vector<Component*> mVectorQ;
    vector<Node*> mVectorN;
    size_t mNumSimSteps;
    double mTimeStep;
    double mTime;
#ifdef Q_OS_OSX
    /* double *mpSimTime; Never used on mac. /magse */
#else
    double *mpSimTime;
#endif
    size_t mnThreads;
    size_t mThreadID;
    BarrierLock *mpBarrier_S;
    BarrierLock *mpBarrier_C;
    BarrierLock *mpBarrier_Q;
    BarrierLock *mpBarrier_N;
};


//! @brief Class for master simulation thread, that is responsible for synchronizing the simulation
class taskSimMaster
{
public:

    //! @brief Constructor for master simulation thread class.
    //! @param sVector Vector with signal components executed from this thread
    //! @param cVector Vector with C-type components executed from this thread
    //! @param qVector Vector with Q-type components executed from this thread
    //! @param nVector Vector with nodes which is logged from this thread
    //! @param *pSimtime Pointer to the simulation time in the component system
    //! @param startTime Start time of simulation
    //! @param timeStep Step time of simulation
    //! @param stopTime Stop Time of simulation
    //! @param nThreads Number of threads used in simulation
    //! @param threadID Number of this thread
    //! @param *pBarrier_S Pointer to barrier before signal components
    //! @param *pBarrier_C Pointer to barrier before C-type components
    //! @param *pBarrier_Q Pointer to barrier before Q-type components
    //! @param *pBarrier_N Pointer to barrier before node logging
    taskSimMaster(ComponentSystem *pSystem, vector<Component*> sVector, vector<Component*> cVector, vector<Component*> qVector, vector<Node*> nVector, vector<double *> pSimTimes,
                  double startTime, double timeStep, size_t numSimSteps, size_t nThreads, size_t threadID,
                  BarrierLock *pBarrier_S, BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N)
    {
        mpSystem = pSystem;
        mVectorS = sVector;
        mVectorC = cVector;
        mVectorQ = qVector;
        mVectorN = nVector;
        mpSimTimes = pSimTimes;
        mTime = startTime;
        mNumSimSteps = numSimSteps;
        mTimeStep = timeStep;
        mnThreads = nThreads;
        mThreadID = threadID;
        mpBarrier_S = pBarrier_S;
        mpBarrier_C = pBarrier_C;
        mpBarrier_Q = pBarrier_Q;
        mpBarrier_N = pBarrier_N;
    }

    //! @brief Executable code for master simulation thread
    void operator() ()
    {
        for(size_t s=0; s<mNumSimSteps; ++s)
        {
            if(mpSystem->wasSimulationAborted()) break;

            mTime += mTimeStep;

            //! Signal Components !//

            while(!mpBarrier_S->allArrived()) {}    //Wait for all other threads to arrive at signal barrier
            mpBarrier_C->lock();                    //Lock next barrier (must be done before unlocking this one, to prevent deadlocks)
            mpBarrier_S->unlock();                  //Unlock signal barrier

            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime);
            }

            //! C Components !//

            while(!mpBarrier_C->allArrived()) {}    //C barrier
            mpBarrier_Q->lock();
            mpBarrier_C->unlock();

            for(size_t i=0; i<mVectorC.size(); ++i)
            {
                mVectorC[i]->simulate(mTime);
            }

            //! Q Components !//

            while(!mpBarrier_Q->allArrived()) {}    //Q barrier
            mpBarrier_N->lock();
            mpBarrier_Q->unlock();

            for(size_t i=0; i<mVectorQ.size(); ++i)
            {
                mVectorQ[i]->simulate(mTime);
            }

            for(size_t i=0; i<mpSimTimes.size(); ++i)
                *mpSimTimes[i] = mTime;     //Update time in component system, so that progress bar can use it

            //! Log Nodes !//

            while(!mpBarrier_N->allArrived()) {}    //N barrier
            mpBarrier_S->lock();
            mpBarrier_N->unlock();

            //! @todo Temporary hack by Peter, after rewriting how node data and time is logged this no longer works, now master thread loags all nodes, need to come up with something smart
//            for(size_t i=0; i<mVectorN.size(); ++i)
//            {
//                mVectorN[i]->logData(mTime);
//            }
            mpSystem->logTimeAndNodes(s+1); //s+1 since at s=0 one simulation has been performed /Björn

        }
    }
private:
    ComponentSystem *mpSystem;
    vector<Component*> mVectorS;
    vector<Component*> mVectorC;
    vector<Component*> mVectorQ;
    vector<Node*> mVectorN;
    size_t mNumSimSteps;
    double mTimeStep;
    double mTime;
    vector<double *> mpSimTimes;
#ifdef Q_OS_OSX
    /* size_t mnSystems; Never used on mac. /magse */
#else
    size_t mnSystems;
#endif
    size_t mnThreads;
    size_t mThreadID;
    BarrierLock *mpBarrier_S;
    BarrierLock *mpBarrier_C;
    BarrierLock *mpBarrier_Q;
    BarrierLock *mpBarrier_N;
};



//! @brief Class for simulating whole systems multi threaded
class taskSimWholeSystems
{
public:

    //! @brief Constructor for master simulation thread class.
    //! @param pSystem Pointer to the system to simulate
    taskSimWholeSystems(vector<ComponentSystem *> systemPtrs, double stopTime)
    {
        mSystemPtrs = systemPtrs;
        mStopTime = stopTime;
    }

    //! @brief Executable code for master simulation thread
    void operator() ()
    {
        for(size_t i=0; i<mSystemPtrs.size(); ++i)
        {
            mSystemPtrs[i]->simulate(mStopTime);
        }
    }
private:
    vector<ComponentSystem *> mSystemPtrs;
    double mStopTime;
};








////////////////////////////////////////
//// Component-Threads Implementation //
////////////////////////////////////////

////! @brief Class for simulating whole systems multi threaded
//class taskSimComponentC
//{
//public:

//    //! @brief Constructor for master simulation thread class.
//    //! @param pSystem Pointer to the system to simulate
//    taskSimComponentC(Component* pComp, vector<taskSimComponent *> neighbourPtrs, size_t stopIteration, bool *pStart, double timeStep)
//    {
//        mpComp = pComp;
//        mIsCtype = (pComp->getTypeCQSString() == "C");
//        mNeighbourPtrs = neighbourPtrs;
//        mStopIteration = stopIteration;
//        mpStart = pStart;
//        mTimeStep = timeStep;
//        mnNeighbours = mNodePtrs.size();
//    }

//    //! @brief Executable code for master simulation thread
//    void operator() ()
//    {
//        while(!pStart) {}

//        double time=0;

//        for(iteration=0; mIter<mStopIteration; ++mIter)
//        {
//            while(!checkNeighbours()) {}

//            time += mTimeStep;
//            mpComp->simulate(time);
//        }
//    }

//    size_t mIter;

//private:
//    bool checkNeighbours()
//    {
//        for(size_t n=0; n<mnNeighbours; ++n)
//        {
//            if(mNeighbourPtrs[n]->mIter != this->mIter)
//                return false;
//        }
//        return true;
//    }

//    Component *mpComp;
//    vector<taskSimComponent *> mNeighbourPtrs;
//    size_t mStopIteration;
//    bool *mpStart;
//    double mTimeStep;
//    size_t mnNeighbours;
//    bool mIsCtype;
//    bool mIsStype;
//};



////! @brief Class for simulating whole systems multi threaded
//class taskSimNode
//{
//public:

//    //! @brief Constructor for master simulation thread class.
//    //! @param pSystem Pointer to the system to simulate
//    taskSimComponent(vector<ComponentSystem *> systemPtrs, double stopTime)
//    {
//        mSystemPtrs = systemPtrs;
//        mStopTime = stopTime;
//    }

//    //! @brief Executable code for master simulation thread
//    void operator() ()
//    {
//        for(size_t i=0; i<mSystemPtrs.size(); ++i)
//        {
//            mSystemPtrs[i]->simulate(mStopTime);
//        }
//    }
//private:
//    vector<ComponentSystem *> mSystemPtrs;
//    double mStopTime;
//};


#ifdef USETBB

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
        size_t idx = mCurrentIdx.fetch_and_increment();
        if(idx >= mSize)
            return 0;
        else
            return mComponentPtrs[idx];
    }

    void reportDone()
    {
        mnDone.fetch_and_increment();
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
        mOpen.fetch_and_store(false);
    }

    bool isOpen()
    {
        return mOpen;
    }

private:
    std::vector<Component*> mComponentPtrs;
    size_t mSize;
    tbb::atomic<size_t> mCurrentIdx;
    tbb::atomic<size_t> mnDone;
    tbb::atomic<bool> mOpen;
};


//! @brief Class for slave simulation threads, which must be synchronized from a master simulation thread
class taskSimPoolSlave
{
public:
    taskSimPoolSlave(TaskPool *pTaskPoolC, TaskPool *pTaskPoolQ, tbb::atomic<double> *pTime, tbb::atomic<bool> *pStop)
    {
        mpTaskPoolC = pTaskPoolC;
        mpTaskPoolQ = pTaskPoolQ;
        mpStop = pStop;
        mpTime = pTime;
    }

    void operator() ()
    {
        Component *pComp;
        while(!(*mpStop))
        //for(size_t i=0; i<mnSteps && !(*mpStop); ++i)
        {
            //C-pool
            if(mpTaskPoolC->isOpen())
            {
                pComp = mpTaskPoolC->getComponent();
                while(pComp)
                {
                    pComp->simulate(*mpTime);
                    mpTaskPoolC->reportDone();
                    pComp = mpTaskPoolC->getComponent();
                }
                while(mpTaskPoolC->isOpen()) {}
            }

            //Q-pool
            if(mpTaskPoolQ->isOpen())
            {
                pComp = mpTaskPoolQ->getComponent();
                while(pComp)
                {
                    pComp->simulate(*mpTime);
                    mpTaskPoolQ->reportDone();
                    pComp = mpTaskPoolQ->getComponent();
                }
                while(mpTaskPoolQ->isOpen()) {}
            }
        }
    }

private:
    TaskPool *mpTaskPoolC;
    TaskPool *mpTaskPoolQ;
    tbb::atomic<bool> *mpStop;
    tbb::atomic<double> *mpTime;
};


/////////////////////////////////////
// Random Task Pool Implementation //
/////////////////////////////////////


class RandomTaskPool
{
public:
    RandomTaskPool(std::vector<Component*> componentPtrs)
    {
        mFinishedComponentPtrs = componentPtrs;
        mSize = componentPtrs.size();
        mnDone = 0;
        mOpen=false;
        mpMutex = new tbb::mutex();
    }

    Component *getComponent()
    {
        Component *pRetVal;
        mpMutex->lock();
        size_t compSize = mComponentPtrs.size();
        if(compSize == 0)
            pRetVal=0;
        else
        {
            size_t random = (size_t)rand() / (size_t)RAND_MAX * compSize;
            pRetVal = mComponentPtrs[random];
            mFinishedComponentPtrs.push_back(mComponentPtrs[random]);
            mComponentPtrs.erase(mComponentPtrs.begin()+random);
        }
        mpMutex->unlock();
        return pRetVal;
    }

    void reportDone()
    {
        ++mnDone;
    }

    bool isReady()
    {
        return mnDone >= mSize;
    }

    void open()
    {
        mComponentPtrs=mFinishedComponentPtrs;
        mFinishedComponentPtrs.clear();
        mnDone.store(0);
        mOpen.store(true);
    }

    void close()
    {
        mOpen=false;
    }

    bool isOpen()
    {
        return mOpen;
    }

private:
    std::vector<Component*> mComponentPtrs;
    std::vector<Component*> mFinishedComponentPtrs;
    size_t mSize;
    tbb::atomic<size_t> mnDone;
    tbb::atomic<bool> mOpen;
   tbb::mutex *mpMutex;
};


//! @brief Class for slave simulation threads, which must be synchronized from a master simulation thread
class taskSimRandomPoolSlave
{
public:
    taskSimRandomPoolSlave(RandomTaskPool *pTaskPoolC, RandomTaskPool *pTaskPoolQ, tbb::atomic<double> *pTime, tbb::atomic<bool> *pStop)
    {
        mpTaskPoolC = pTaskPoolC;
        mpTaskPoolQ = pTaskPoolQ;
        mpStop = pStop;
        mpTime = pTime;
    }

    void operator() ()
    {
        Component *pComp;
        while(!(*mpStop))
        //for(size_t i=0; i<mnSteps && !(*mpStop); ++i)
        {
            //C-pool
            if(mpTaskPoolC->isOpen())
            {
                pComp = mpTaskPoolC->getComponent();
                while(pComp)
                {
                    pComp->simulate(*mpTime);
                    mpTaskPoolC->reportDone();
                    pComp = mpTaskPoolC->getComponent();
                }
                while(mpTaskPoolC->isOpen()) {}
            }

            //Q-pool
            if(mpTaskPoolQ->isOpen())
            {
                pComp = mpTaskPoolQ->getComponent();
                while(pComp)
                {
                    pComp->simulate(*mpTime);
                    mpTaskPoolQ->reportDone();
                    pComp = mpTaskPoolQ->getComponent();
                }
                while(mpTaskPoolQ->isOpen()) {}
            }
        }
    }

private:
    RandomTaskPool *mpTaskPoolC;
    RandomTaskPool *mpTaskPoolQ;
    tbb::atomic<bool> *mpStop;
    tbb::atomic<double> *mpTime;
};

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
        mpMutex = new tbb::mutex();
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
    tbb::mutex *mpMutex;
};



//! @brief Class for master simulation thread, that is responsible for synchronizing the simulation
class taskSimStealingMaster
{
public:
    taskSimStealingMaster(ComponentSystem *pSystem, vector<Component*> sVector, std::vector<ThreadSafeVector*> *cVectors, std::vector<ThreadSafeVector*> *qVectors,
                          vector<double *> pSimTimes, double startTime, double timeStep, size_t numSimSteps, size_t nThreads, size_t threadID,
                          BarrierLock *pBarrier_S, BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N, size_t maxSize)
    {
        mpSystem = pSystem;
        mVectorS = sVector;
        mpUnFinishedVectorsC = cVectors;
        mpUnFinishedVectorsQ = qVectors;
        mpFinishedVectorC = new ThreadSafeVector(std::vector<Component*>(), maxSize);
        mpFinishedVectorQ = new ThreadSafeVector(std::vector<Component*>(), maxSize);
        mpSimTimes = pSimTimes;
        mTime = startTime;
        mNumSimSteps = numSimSteps;
        mTimeStep = timeStep;
        mnThreads = nThreads;
        mThreadID = threadID;
        mpBarrier_S = pBarrier_S;
        mpBarrier_C = pBarrier_C;
        mpBarrier_Q = pBarrier_Q;
        mpBarrier_N = pBarrier_N;
    }

    //! @brief Executable code for master simulation thread
    void operator() ()
    {
        ThreadSafeVector *pTemp;
        Component *pComp;

        for(size_t s=0; s<mNumSimSteps; ++s)
        {
            if(mpSystem->wasSimulationAborted()) break;

            mTime += mTimeStep;

            //! Signal Components !//

            while(!mpBarrier_S->allArrived()) {}
            mpBarrier_C->lock();
            mpBarrier_S->unlock();

            //Simulate signal components
            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime);
            }

            //! C Components !//

            while(!mpBarrier_C->allArrived()) {}    //C barrier
//            std::cout << "------------------------------------------------------------\n";
//            std::cout << "Time: " << mTime << "\n";
//            std::cout << "---------------------------------------\n";
//            std::cout << "C-type components\n";
//            for(size_t i=0; i<mnThreads; ++i)
//            {
//                std::cout << "Thread " << i << ": \n";
//                for(size_t j=0; j<mpUnFinishedVectorsC->at(i)->size(); ++j)
//                {
//                    Component *pTempComponent = mpUnFinishedVectorsC->at(i)->at(j);
//                    if(pTempComponent)
//                        std::cout << "  " << pTempComponent->getName().c_str() << "\n";
//                }
//            }
            mpBarrier_Q->lock();
            mpBarrier_C->unlock();

            //SIMULATE C

            //Switch Q vectors
            pTemp = mpUnFinishedVectorsQ->at(mThreadID);
            mpUnFinishedVectorsQ->at(mThreadID) = mpFinishedVectorQ;
            mpFinishedVectorQ = pTemp;

            //Simulate own components
            pComp = mpUnFinishedVectorsC->at(mThreadID)->tryAndTakeFirst();
            while(pComp)
            {
                pComp->simulate(mTime);
                mpFinishedVectorC->insertFirst(pComp);
                pComp = mpUnFinishedVectorsC->at(mThreadID)->tryAndTakeFirst();
            }

            //Steal components
            //if(int(mTime/mTimeStep/5+mThreadID)%mnThreads == 0)
            {
                for(size_t i=0; i<mnThreads-1; ++i)
                {
                    int j = (mThreadID+1+i)%mnThreads;
                    pComp = mpUnFinishedVectorsC->at(j)->tryAndTakeLast();
                    if(pComp)
                    {
                        pComp->simulate(mTime);
                        mpFinishedVectorC->insertLast(pComp);
                        break;
                    }
                }
            }

            //! Q Components !//

            while(!mpBarrier_Q->allArrived()) {}    //Q barrier
//            std::cout << "------------------------------------------------------------\n";
//            std::cout << "Time: " << mTime << "\n";
//            std::cout << "---------------------------------------\n";
//            std::cout << "Q-type components\n";
//            for(size_t i=0; i<mnThreads; ++i)
//            {
//                std::cout << "Thread " << i << ": \n";
//                for(size_t j=0; j<mpUnFinishedVectorsQ->at(i)->size(); ++j)
//                {
//                    Component *pTempComponent = mpUnFinishedVectorsQ->at(i)->at(j);
//                    if(pTempComponent)
//                        std::cout << "  " << pTempComponent->getName().c_str() << "\n";
//                }
//            }
            mpBarrier_N->lock();
            mpBarrier_Q->unlock();

            //SIMULATE Q

            //Switch C vectors
            pTemp = mpUnFinishedVectorsC->at(mThreadID);
            mpUnFinishedVectorsC->at(mThreadID) = mpFinishedVectorC;
            mpFinishedVectorC = pTemp;

            //Simulate own components
            pComp = mpUnFinishedVectorsQ->at(mThreadID)->tryAndTakeFirst();
            while(pComp)
            {
                pComp->simulate(mTime);
                mpFinishedVectorQ->insertFirst(pComp);
                pComp = mpUnFinishedVectorsQ->at(mThreadID)->tryAndTakeFirst();
            }

            //Steal components
            //if(int(mTime/mTimeStep/5+mThreadID)%mnThreads == 0)
            {
                for(size_t i=0; i<mnThreads-1; ++i)
                {
                    int j = (mThreadID+1+i)%mnThreads;
                    pComp = mpUnFinishedVectorsQ->at(j)->tryAndTakeLast();
                    if(pComp)
                    {
                        pComp->simulate(mTime);
                        mpFinishedVectorQ->insertLast(pComp);
                        break;
                    }
                }
            }

            for(size_t i=0; i<mpSimTimes.size(); ++i)
            *mpSimTimes[i] = mTime;

            //! Log Nodes !//

            while(!mpBarrier_N->allArrived()) {}    //N barrier
            mpBarrier_S->lock();
            mpBarrier_N->unlock();

            mpSystem->logTimeAndNodes(s+1);

        }
    }
private:
    ComponentSystem *mpSystem;
    vector<Component*> mVectorS;
    ThreadSafeVector *mpFinishedVectorC;
    ThreadSafeVector *mpFinishedVectorQ;
    std::vector<ThreadSafeVector*> *mpUnFinishedVectorsC;
    std::vector<ThreadSafeVector*> *mpUnFinishedVectorsQ;
    vector<Node*> mVectorN;
    size_t mNumSimSteps;
    double mTimeStep;
    double mTime;
    vector<double *> mpSimTimes;
    size_t mnThreads;
    size_t mThreadID;
    BarrierLock *mpBarrier_S;
    BarrierLock *mpBarrier_C;
    BarrierLock *mpBarrier_Q;
    BarrierLock *mpBarrier_N;
};



class taskSimStealingSlave
{
public:
    taskSimStealingSlave(ComponentSystem *pSystem, std::vector<ThreadSafeVector*> *cVectors, std::vector<ThreadSafeVector*> *qVectors,
                         double startTime, double timeStep, size_t numSimSteps, size_t nThreads, size_t threadID,
                         BarrierLock *pBarrier_S, BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N, size_t maxSize)
    {
        mpSystem = pSystem;
        mpUnFinishedVectorsC = cVectors;
        mpUnFinishedVectorsQ = qVectors;
        mpFinishedVectorC = new ThreadSafeVector(std::vector<Component*>(), maxSize);
        mpFinishedVectorQ = new ThreadSafeVector(std::vector<Component*>(), maxSize);
        mTime = startTime;
        mNumSimSteps = numSimSteps;
        mTimeStep = timeStep;
        mnThreads = nThreads;
        mThreadID = threadID;
        mpBarrier_S = pBarrier_S;
        mpBarrier_C = pBarrier_C;
        mpBarrier_Q = pBarrier_Q;
        mpBarrier_N = pBarrier_N;
    }

    void operator() ()
    {
        ThreadSafeVector *pTemp;
        Component *pComp;

        for(size_t i=0; i<mNumSimSteps; ++i)
        {
            if(mpSystem->wasSimulationAborted()) break;

            mTime += mTimeStep;

            //! Signal Components !//

            mpBarrier_S->increment();
            while(mpBarrier_S->isLocked()){}                         //Wait at S barrier

            //! C Components !//

            mpBarrier_C->increment();
            while(mpBarrier_C->isLocked()){}                         //Wait at C barrier

            //C-COMPONENTS

            //Switch Q vectors
            pTemp = mpUnFinishedVectorsQ->at(mThreadID);
            mpUnFinishedVectorsQ->at(mThreadID) = mpFinishedVectorQ;
            mpFinishedVectorQ = pTemp;

            //Simulate own components
            pComp = mpUnFinishedVectorsC->at(mThreadID)->tryAndTakeFirst();
            while(pComp)
            {
                pComp->simulate(mTime);
                mpFinishedVectorC->insertFirst(pComp);
                pComp = mpUnFinishedVectorsC->at(mThreadID)->tryAndTakeFirst();
            }

            //Steal components
            //if(int(mTime/mTimeStep/5+mThreadID)%mnThreads == 0)
            {
                for(size_t i=0; i<mnThreads-1; ++i)
                {
                    int j = (mThreadID+1+i)%mnThreads;
                    pComp = mpUnFinishedVectorsC->at(j)->tryAndTakeLast();
                    if(pComp)
                    {
                        pComp->simulate(mTime);
                        mpFinishedVectorC->insertLast(pComp);
                        break;
                    }
                }
            }

            //! Q Components !//

            mpBarrier_Q->increment();
            while(mpBarrier_Q->isLocked()){}                         //Wait at Q barrier

            //Q-COMPONENTS

            //Switch C vectors
            pTemp = mpUnFinishedVectorsC->at(mThreadID);
            mpUnFinishedVectorsC->at(mThreadID) = mpFinishedVectorC;
            mpFinishedVectorC = pTemp;

            //Simulate own components
            pComp = mpUnFinishedVectorsQ->at(mThreadID)->tryAndTakeFirst();
            while(pComp)
            {
                pComp->simulate(mTime);
                mpFinishedVectorQ->insertFirst(pComp);
                pComp = mpUnFinishedVectorsQ->at(mThreadID)->tryAndTakeFirst();
            }

            //Steal components
            //if(int(mTime/mTimeStep/5+mThreadID)%mnThreads == 0)
            {
                for(size_t i=0; i<mnThreads-1; ++i)
                {
                    int j = (mThreadID+1+i)%mnThreads;
                    pComp = mpUnFinishedVectorsQ->at(j)->tryAndTakeLast();
                    if(pComp)
                    {
                        pComp->simulate(mTime);
                        mpFinishedVectorQ->insertLast(pComp);
                        break;
                    }
                }
            }

            //! Log Nodes !//

            mpBarrier_N->increment();
            while(mpBarrier_N->isLocked()){}                         //Wait at N barrier
        }
    }
private:
    ComponentSystem *mpSystem;
    ThreadSafeVector *mpFinishedVectorC;
    ThreadSafeVector *mpFinishedVectorQ;
    std::vector<ThreadSafeVector*> *mpUnFinishedVectorsC;
    std::vector<ThreadSafeVector*> *mpUnFinishedVectorsQ;
    size_t mNumSimSteps;
    double mTimeStep;
    double mTime;
    size_t mThreadID;
    size_t mnThreads;
    BarrierLock *mpBarrier_S;
    BarrierLock *mpBarrier_C;
    BarrierLock *mpBarrier_Q;
    BarrierLock *mpBarrier_N;
};


/////////////////////////////////////////////
// Parallel for loop algorithm using tasks //
/////////////////////////////////////////////

class TaskSimOneComponentOneStep
{
public:
    TaskSimOneComponentOneStep(Component * pComp, double stopTime)
    {
        mpComp = pComp;
        mStopTime = stopTime;
    }

    void operator() ()
    {
        mpComp->simulate(mStopTime);
    }

private:
    Component *mpComp;
    double mStopTime;
};


/////////////////////////////////////////////
// Parallel for loop algorithm using tasks //
/////////////////////////////////////////////

class TaskSimOneStep
{
public:
    TaskSimOneStep(std::vector<Component *> *pComponentPtrs, double stopTime)
    {
        mpComponentPtrs = pComponentPtrs;
        mStopTime = stopTime;
        nComponents = mpComponentPtrs->size();
    }

    void operator() ()
    {
        for (size_t i=0; i<nComponents; ++i)
        {
            mpComponentPtrs->at(i)->simulate(mStopTime);
        }
    }

private:
    std::vector<Component *> *mpComponentPtrs;
    double mStopTime;
    size_t nComponents;
};

////////////////////////////////////////////////
// Parallel for algorithm using TBB templates //
////////////////////////////////////////////////

class BodySimulateComponentVector
{
public:
    BodySimulateComponentVector(std::vector<Component *> *pComponentPtrs, double stopTime)
    {
        mpComponentPtrs = pComponentPtrs;
        mStopTime = stopTime;
    }

    void operator() ( const tbb::blocked_range<int>& r ) const
    {
        for (int i=r.begin(); i!=r.end(); ++i)
        {
            mpComponentPtrs->at(i)->simulate(mStopTime);
        }
    }

private:
    std::vector<Component *> *mpComponentPtrs;
    double mStopTime;
};



#endif // USETBB

#endif // MULTITHREADINGUTILITIES_H
