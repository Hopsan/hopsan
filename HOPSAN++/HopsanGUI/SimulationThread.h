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

class ProjectTab;
class CoreSystemAccess;

class SimulationThread : public QThread
{
public:
    SimulationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, ProjectTab *parent);

    ProjectTab *mpParentProjectTab;
    CoreSystemAccess *mpGUIRootSystem;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;

};

#endif // SIMULATIONTHREAD_H
