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

//Forward Declaration
class CoreSystemAccess;

class SimulationThread : public QThread
{
public:
    SimulationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, QObject *parent);

protected:
    void run();

private:
    CoreSystemAccess *mpCoreSystemAccess;
    double mStartTime;
    double mFinishTime;

};

#endif // SIMULATIONTHREAD_H
