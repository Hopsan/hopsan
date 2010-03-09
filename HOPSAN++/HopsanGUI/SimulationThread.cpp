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


SimulationThread::SimulationThread(ComponentSystem *pComponentSystem, double startTime, double finishTime, ProjectTabWidget *parent)
{
    mpParentProjectTabWidget = parent;
    mpComponentSystem = pComponentSystem;
    mStartTime = startTime;
    mFinishTime = finishTime;

}

void SimulationThread::run()
{
    mpComponentSystem->initialize(mStartTime, mFinishTime);
    mpComponentSystem->simulate(mStartTime, mFinishTime);








//    simulate
    //exec();
}
