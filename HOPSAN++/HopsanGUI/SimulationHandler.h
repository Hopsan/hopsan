/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SimulationHandler.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-25
//!
//! @brief Contains a class for running the simulation and signaling a progress bar
//!
//$Id$

#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QThread>
#include <QTimer>
#include <QProgressDialog>

// Forward Declaration
class SystemContainer;


class SimulationObject : public QObject
{
    Q_OBJECT

private:
    SystemContainer *mpSystem;
    double mStartTime;
    double mStopTime;
    unsigned int mnLogSamples;
    QProgressDialog *mpProgressDialog;

public:
    SimulationObject(SystemContainer* pSystem, const double startTime, const double stopTime, const unsigned int nLogSamples)
        : mpSystem(pSystem), mStartTime(startTime), mStopTime(stopTime), mnLogSamples(nLogSamples), mpProgressDialog(0) {}
    void setProgressDialog(QProgressDialog * pProgressDialog);

public slots:
    void doIt();

signals:
    void setProgressBarRange(int, int);
    void setProgressBarText(QString text);
    void initDone(bool, int);
    void simulateDone(bool, int);
    void finalizeDone(bool, int);
};

class SimulationHandler  : public QObject
{
    Q_OBJECT

public:
    SimulationHandler(SystemContainer* pSystem, const double startTime, const double finishTime, const unsigned int nSamples);
    bool wasSuccessful();

    void setSystem(SystemContainer* pSystem);
    void setSimulationTime( const double startTime, const double stopTime, const unsigned int nLogSamples);

signals:
    void startSimulation();
    void setProgressBarValue(int);
    void done(bool);

protected slots:
    void initDone(bool success, int ms);
    void simulateDone(bool success, int ms);
    void finalizeDone(bool success, int ms);
    void abort();
    void refreshProgressBar();

protected:
    SystemContainer *mpSystem;
    SimulationObject *mpSimulationObject;

    QThread simulationThread;
    QTimer mProgressDialogRefreshTimer;
    QProgressDialog *mpProgressDialog;

    double mStartT, mStopT;
    bool mInitSuccess, mSimuSucess, mFiniSucess, mAborted;
    int mInitTime, mSimuTime, mFiniTime, mLastProgressRefreshStep;
};


#endif // SIMULATIONHANDLER_H
