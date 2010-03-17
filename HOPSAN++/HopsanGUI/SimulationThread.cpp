//!
//! @file   SimulationThread.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-09
//!
//! @brief Contains a class for simulation in a separate class
//!
//$Id$


#include "SimulationThread.h"
#include "HopsanCore.h"
#include "ProjectTabWidget.h"
#include "mainwindow.h"


//! @class SimulationThread
//! @brief The SimulationThread class implement a class to simulate a core model in a separate thread
//!
//! One reason to implement a simulation in a separate thread is to enable a progress bar.
//!


//! Constructor.
//! @param pComponentSystem is a pointer to the system to simulate.
//! @param startTime is the start time for the simulation.
//! @param finishTime is the finish time for the simulation.
//! @param parent is the parent of the thread, the a ProjectTabWidget
SimulationThread::SimulationThread(ComponentSystem *pComponentSystem, double startTime, double finishTime, ProjectTabWidget *parent)
{
    mpParentProjectTabWidget = parent;
    mpComponentSystem = pComponentSystem;
    mStartTime = startTime;
    mFinishTime = finishTime;

}


//! Implements the task for the thread.
void SimulationThread::run()
{
    mpComponentSystem->simulate(mStartTime, mFinishTime);
    mpComponentSystem->finalize(mStartTime, mFinishTime);

    //exec(); //Is used if one want to run an event loop in this thread.
}
