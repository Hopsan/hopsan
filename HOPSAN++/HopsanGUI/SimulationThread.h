//!
//! @file   SimulationThread.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-09
//!
//! @brief Contains a class for simulation in a separate class
//!
//$Id$


#ifndef SIMULATIONTHREAD_H
#define SIMULATIONTHREAD_H

#include <QThread>


class ComponentSystem;
class ProjectTabWidget;

class SimulationThread : public QThread
{
public:
    SimulationThread(ComponentSystem *pComponentSystem, double startTime, double finishTime, ProjectTabWidget *parent);

    ProjectTabWidget *mpParentProjectTabWidget;
    ComponentSystem *mpComponentSystem;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;

};

#endif // SIMULATIONTHREAD_H
