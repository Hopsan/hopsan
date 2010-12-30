//!
//! @file   SimulationThread.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-09
//!
//! @brief Contains a class for simulation in a separate class
//!
//$Id$


#include "SimulationThread.h"
#include "CoreAccess.h"
#include "Configuration.h"
#include "common.h"


//! @class SimulationThread
//! @brief The SimulationThread class implement a class to simulate a core model in a separate thread
//!
//! One reason to implement a simulation in a separate thread is to enable a progress bar.
//!


//! @brief Constructor for simulation thread
//! @param pComponentSystem Pointer to the system to simulate.
//! @param startTime Start time for the simulation.
//! @param finishTime Finish time for the simulation.
//! @param parent Parent of the thread, the ProjectTab (Automatically casted to QObject*)
SimulationThread::SimulationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, QObject *parent)
    : QThread(parent)
{
    mpCoreSystemAccess = pGUIRootSystem;

    mStartTime = startTime;
    mFinishTime = finishTime;
}


//! @brief Implements the task for the thread.
void SimulationThread::run()
{
    if(gConfig.getUseMulticore())
    {
        mpCoreSystemAccess->simulate(mStartTime, mFinishTime, MULTICORE, gConfig.getNumberOfThreads());
    }
    else
    {
        mpCoreSystemAccess->simulate(mStartTime, mFinishTime, SINGLECORE);
    }
    mpCoreSystemAccess->finalize(mStartTime, mFinishTime);

    //exec(); //Is used if one want to run an event loop in this thread.
}
