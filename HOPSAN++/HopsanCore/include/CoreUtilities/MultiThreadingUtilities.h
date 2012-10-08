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
#include "mutex.h"
#include "atomic.h"
#include "tick_count.h"
#include "task_group.h"
#endif

#include "Component.h"
#include "ComponentSystem.h"

using namespace std;
using namespace hopsan;

size_t determineActualNumberOfThreads(const size_t nDesiredThreads);


template <class T>
class TaskPool
{
public:
    TaskPool(std::vector<T*> data, int nThreads)
    {
        mData = data;
        mStartIdx=mData.size()-1;
        std::vector<T*> tempData;
        for(int i=0; i<mData.size(); ++i)
        {
            tempData.push_back(mData[i]);
            tempData.push_back(0);
            tempData.push_back(0);
            tempData.push_back(0);
        }
        mData = tempData;
        mnThreads=nThreads;
        reset();
//#ifdef USETBB
//        mpGetMutex = new tbb::mutex();
//#else
//        mpGetMutex = 0;
//#endif
    }

    inline void reset()
    {
        mIdx = mStartIdx;
    }


    inline bool isFinished()
    {
        return mIdx <= -mnThreads;
    }


    inline int getAndDecrementIndex()
    {
        return mIdx.fetch_and_store(mIdx-1);
    }


    inline T* getDataPtr(int idx)
    {
        return mData[idx*4];
    }

private:
    int mStartIdx;
    std::vector<T*> mData;
    //tbb::mutex *mpGetMutex;
    tbb::atomic<int> mIdx;
    int mnThreads;
};



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
    int mLock;
#endif
};




//! @brief Class for slave simulation threads, which must be syncronized from a master simulation thread
class taskSimOneComponent
{
public:
    taskSimOneComponent(Component* pComponent, double time, double nextTime)
    {
        mpComponent = pComponent;
        mTime = time;
        mNextTime = nextTime;
    }

    void operator() ()
    {
        mpComponent->simulate(mTime, mNextTime);
    }
private:
    Component *mpComponent;
    double mTime;
    double mNextTime;
};




//! @brief Class for slave simulation threads, which must be syncronized from a master simulation thread
class taskSimPool
{
public:
    taskSimPool(TaskPool<Component> *sPool, TaskPool<Component> *qPool, TaskPool<Component> *cPool, TaskPool<Node> *nPool,
                double startTime, double timeStep, double stopTime, int threadId, ComponentSystem* pSystem)
    {
        mTime = startTime;
        mStopTime = stopTime;
        mTimeStep = timeStep;
        msPool = sPool;
        mqPool = qPool;
        mcPool = cPool;
        mnPool = nPool;
        mThreadId = threadId;
        mpSystem = pSystem;
    }

    void operator() ()
    {
        while(mTime < mStopTime)
        {
            int idx;
            double nextTime = mTime+mTimeStep;

            //! Signal Components !//

            idx = msPool->getAndDecrementIndex();
            while(idx>=0)
            {
                Component *pComponent = msPool->getDataPtr(idx);
                pComponent->simulate(mTime, nextTime);
                idx = msPool->getAndDecrementIndex();
            }
            while(!msPool->isFinished()) {}
            if(mThreadId == 0) { mnPool->reset(); }

            //! C Components !//

            idx = mcPool->getAndDecrementIndex();
            while(idx>=0)
            {
                Component *pComponent = mcPool->getDataPtr(idx);
                pComponent->simulate(mTime, nextTime);
                idx = mcPool->getAndDecrementIndex();
            }
            while(!mcPool->isFinished()) {}
            if(mThreadId == 0) { mqPool->reset(); }

            //! Log Nodes !//

            if(mThreadId == 0)      //Hack because of Peters log data changes, must be fixed
            {
                mpSystem->logTimeAndNodes(mTime);
            }
            idx = mnPool->getAndDecrementIndex();
            while(idx>=0)
            {
                idx = mnPool->getAndDecrementIndex();
            }
            while(!mnPool->isFinished()) {}
            if(mThreadId == 0) { msPool->reset(); }

            //! Q Components !//

            idx = mqPool->getAndDecrementIndex();
            while(idx>=0)
            {
                Component *pComponent = mqPool->getDataPtr(idx);
                pComponent->simulate(mTime, nextTime);
                idx = mqPool->getAndDecrementIndex();
            }
            while(!mqPool->isFinished()) {}
            if(mThreadId == 0) { mcPool->reset(); }

            mTime += mTimeStep;
        }
    }
private:
    double mStopTime;
    double mTimeStep;
    double mTime;
    TaskPool<Component> *msPool;
    TaskPool<Component> *mqPool;
    TaskPool<Component> *mcPool;
    TaskPool<Node> *mnPool;
    int mThreadId;
    ComponentSystem *mpSystem;
};




//! @brief Class for slave simulation threads, which must be syncronized from a master simulation thread
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
    taskSimSlave(vector<Component*> sVector, vector<Component*> cVector, vector<Component*> qVector, vector<Node*> nVector,
                 double startTime, double timeStep, double stopTime, size_t nThreads, size_t threadID,
                 BarrierLock *pBarrier_S, BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N)
    {
        mVectorS = sVector;
        mVectorC = cVector;
        mVectorQ = qVector;
        mVectorN = nVector;
        mTime = startTime;
        mStopTime = stopTime;
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
        while(mTime < mStopTime)
        {

            //! Signal Components !//

            mpBarrier_S->increment();
            while(mpBarrier_S->isLocked()){}                         //Wait at S barrier

            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime, mTime+mTimeStep);
            }


            //! C Components !//

            mpBarrier_C->increment();
            while(mpBarrier_C->isLocked()){}                         //Wait at C barrier

            for(size_t i=0; i<mVectorC.size(); ++i)
            {
                mVectorC[i]->simulate(mTime, mTime+mTimeStep);
            }

            //! Log Nodes !//

            mpBarrier_N->increment();
            while(mpBarrier_N->isLocked()){}                         //Wait at N barrier

            //! @todo Temporary hack by Peter, after rewriting how node data and time is logged this no longer works, now master thread loags all nodes, need to come up with somthing smart
//            for(size_t i=0; i<mVectorN.size(); ++i)
//            {
//                mVectorN[i]->logData(mTime);
//            }

            //! Q Components !//

            mpBarrier_Q->increment();
            while(mpBarrier_Q->isLocked()){}                         //Wait at Q barrier

            for(size_t i=0; i<mVectorQ.size(); ++i)
            {
                mVectorQ[i]->simulate(mTime, mTime+mTimeStep);
            }

            mTime += mTimeStep;
        }
    }
private:
    vector<Component*> mVectorS;
    vector<Component*> mVectorC;
    vector<Component*> mVectorQ;
    vector<Node*> mVectorN;
    double mStopTime;
    double mTimeStep;
    double mTime;
    double *mpSimTime;
    size_t mnThreads;
    size_t mThreadID;
    BarrierLock *mpBarrier_S;
    BarrierLock *mpBarrier_C;
    BarrierLock *mpBarrier_Q;
    BarrierLock *mpBarrier_N;
};


//! @brief Class for master simulation thread, that is responsible for syncronizing the simulation
class taskSimMaster
{
public:

    //! @brief Constructor for master simulation thead class.
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
    taskSimMaster(vector<Component*> sVector, vector<Component*> cVector, vector<Component*> qVector, vector<Node*> nVector, vector<double *> pSimTimes,
                  double startTime, double timeStep, double stopTime, size_t nThreads, size_t threadID,
                  BarrierLock *pBarrier_S, BarrierLock *pBarrier_C, BarrierLock *pBarrier_Q, BarrierLock *pBarrier_N)
    {
        mpSystem = nVector[0]->getOwnerSystem();
        mVectorS = sVector;
        mVectorC = cVector;
        mVectorQ = qVector;
        mVectorN = nVector;
        mpSimTimes = pSimTimes;
        mTime = startTime;
        mStopTime = stopTime;
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
        while(mTime < mStopTime)
        {

            //! Signal Components !//

            while(!mpBarrier_S->allArrived()) {}    //Wait for all other threads to arrive at signal barrier
            mpBarrier_C->lock();                    //Lock next barrier (must be done before unlocking this one, to prevnet deadlocks)
            mpBarrier_S->unlock();                  //Unlock signal barrier


            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime, mTime+mTimeStep);
            }

            //! C Components !//

            while(!mpBarrier_C->allArrived()) {}    //C barrier
            mpBarrier_N->lock();
            mpBarrier_C->unlock();

            for(size_t i=0; i<mVectorC.size(); ++i)
            {
                mVectorC[i]->simulate(mTime, mTime+mTimeStep);
            }

            //! Log Nodes !//

            while(!mpBarrier_N->allArrived()) {}    //N barrier
            mpBarrier_Q->lock();
            mpBarrier_N->unlock();

            //! @todo Temporary hack by Peter, after rewriting how node data and time is logged this no longer works, now master thread loags all nodes, need to come up with somthing smart
//            for(size_t i=0; i<mVectorN.size(); ++i)
//            {
//                mVectorN[i]->logData(mTime);
//            }
            mpSystem->logTimeAndNodes(mTime);

            //! Q Components !//

            while(!mpBarrier_Q->allArrived()) {}    //Q barrier
            mpBarrier_S->lock();
            mpBarrier_Q->unlock();

            for(size_t i=0; i<mVectorQ.size(); ++i)
            {
                mVectorQ[i]->simulate(mTime, mTime+mTimeStep);
            }

            for(size_t i=0; i<mpSimTimes.size(); ++i)
                *mpSimTimes[i] = mTime;     //Update time in component system, so that progress bar can use it

            mTime += mTimeStep;
        }
    }
private:
    ComponentSystem *mpSystem;
    vector<Component*> mVectorS;
    vector<Component*> mVectorC;
    vector<Component*> mVectorQ;
    vector<Node*> mVectorN;
    double mStopTime;
    double mTimeStep;
    double mTime;
    vector<double *> mpSimTimes;
    size_t mnSystems;
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

    //! @brief Constructor for master simulation thead class.
    //! @param pSystem Pointer to the system to simulate
    taskSimWholeSystems(vector<ComponentSystem *> systemPtrs, double startTime, double stopTime)
    {
        mSystemPtrs = systemPtrs;
        mStartTime = startTime;
        mStopTime = stopTime;
    }

    //! @brief Executable code for master simulation thread
    void operator() ()
    {
        for(size_t i=0; i<mSystemPtrs.size(); ++i)
        {
            mSystemPtrs[i]->simulate(mStartTime, mStopTime);
        }
    }
private:
    vector<ComponentSystem *> mSystemPtrs;
    double mStartTime;
    double mStopTime;
};


#endif // MULTITHREADINGUTILITIES_H
