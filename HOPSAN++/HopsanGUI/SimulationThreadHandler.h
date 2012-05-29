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
//! @file   SimulationThreadHandler.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-25
//!
//! @brief Contains a class for running the simulation and signaling a progress bar
//!
//$Id$

#ifndef SIMULATIONTHREADHANDLER_H
#define SIMULATIONTHREADHANDLER_H

#include <QThread>
#include <QTimer>
#include <QProgressDialog>

// Forward Declaration
class SystemContainer;
class SimulationWorkerObject;

class SimulationWorkerObject : public QObject
{
    Q_OBJECT

private:
    QVector<SystemContainer*> mvpSystems;
    double mStartTime;
    double mStopTime;
    unsigned int mnLogSamples;
    bool mNoChanges;

public:
    SimulationWorkerObject(QVector<SystemContainer*> vpSystems, const double startTime, const double stopTime, const unsigned int nLogSamples, const bool noChanges)
        : mvpSystems(vpSystems), mStartTime(startTime), mStopTime(stopTime), mnLogSamples(nLogSamples), mNoChanges(noChanges){}
    void connectProgressDialog(QProgressDialog *pProgressDialog);

public slots:
    void initSimulatFinalize();

signals:
    void setProgressBarRange(int, int);
    void setProgressBarText(QString text);
    void closeProgressBarDialog();
    void initDone(bool, int);
    void simulateDone(bool, int);
    void finalizeDone(bool, int);
};

class ProgressBarWorkerObject : public QObject
{
    Q_OBJECT

private:
    QTimer mProgressDialogRefreshTimer;
    QVector<SystemContainer*> mvSystems;
    int mLastProgressRefreshStep;
    double mStartT, mStopT;

protected slots:
    void refreshProgressBar();
    void abort();

public:
    ProgressBarWorkerObject(const double startTime, const double stopTime, const int refreshTime, const QVector<SystemContainer*> &rvSystems, QProgressDialog *pProgressDialog);

signals:
    void setProgressBarValue(int);
    void aborted();
};

class SimulationThreadHandler  : public QObject
{
    Q_OBJECT

private:
    QVector<SystemContainer*> mvpSystems;

    SimulationWorkerObject *mpSimulationWorkerObject;
    ProgressBarWorkerObject *mpProgressBarWorkerObject;
    QProgressDialog *mpProgressDialog;

    QThread mSimulationWorkerThread;
    QThread mProgressBarWorkerThread;

    double mStartT, mStopT;
    int mnLogSamples;

    bool mInitSuccess, mSimuSucess, mFiniSucess, mAborted;
    int mInitTime, mSimuTime, mFiniTime;

protected slots:
    void initDone(bool success, int ms);
    void simulateDone(bool success, int ms);
    void finalizeDone(bool success, int ms);
    void aborted();

public:
    SimulationThreadHandler() : mpSimulationWorkerObject(0), mpProgressBarWorkerObject(0), mStartT(0), mStopT(1), mnLogSamples(0){}

    void setSimulationTimeVariables(const double startTime, const double stopTime, const unsigned int nLogSamples);
    void initSimulateFinalize(SystemContainer* pSystem, const bool noChanges=false);
    void initSimulateFinalize(QVector<SystemContainer*> vpSystems, const bool noChanges=false);
    bool wasSuccessful();

signals:
    void startSimulation();
    void showProgressBar();
    void done(bool);
};


#endif // SIMULATIONTHREADHANDLER_H
