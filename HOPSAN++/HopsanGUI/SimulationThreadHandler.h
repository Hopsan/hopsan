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
class TerminalWidget;

class SimulationWorkerObject : public QObject
{
    Q_OBJECT

private:
    QVector<SystemContainer*> mvpSystems;
    double mStartTime;
    double mStopTime;
    double mLogStartTime;
    unsigned int mnLogSamples;
    bool mNoChanges;

public:
    SimulationWorkerObject(QVector<SystemContainer*> vpSystems, const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples, const bool noChanges)
        : mvpSystems(vpSystems), mStartTime(startTime), mStopTime(stopTime), mLogStartTime(logStartTime), mnLogSamples(nLogSamples), mNoChanges(noChanges){}
    void connectProgressDialog(QProgressDialog *pProgressDialog);

public slots:
    void initSimulateFinalize();

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

public slots:
    void startRefreshTimer(int ts);

public:
    ProgressBarWorkerObject(const double startTime, const double stopTime, const QVector<SystemContainer*> &rvSystems, QProgressDialog *pProgressDialog);

signals:
    void stopRefreshTimer();
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

    double mStartT, mStopT, mLogStartTime;
    int mnLogSamples;

    bool mInitSuccess, mSimuSucess, mFiniSucess, mAborted, mProgressBarEnabled, mProgressBarModal;
    int mInitTime, mSimuTime, mFiniTime;


protected slots:
    void initDone(bool success, int ms);
    void simulateDone(bool success, int ms);
    void finalizeDone(bool success, int ms);
    void aborted();

public:
    SimulationThreadHandler(TerminalWidget *pTerminal) : mpSimulationWorkerObject(0), mpProgressBarWorkerObject(0), mpProgressDialog(0), mStartT(0), mStopT(1), mnLogSamples(0), mProgressBarEnabled(true), mProgressBarModal(true)
    {
        mpTerminal = pTerminal;
    }

    void setSimulationTimeVariables(const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples);
    void setProgressDilaogBehaviour(bool enabled, bool modal);
    void initSimulateFinalize(SystemContainer* pSystem, const bool noChanges=false);
    void initSimulateFinalize(QVector<SystemContainer*> vpSystems, const bool noChanges=false);
    void initSimulateFinalize_blocking(QVector<SystemContainer*> vpSystems, const bool noChanges=false);
    bool wasSuccessful();
    int getLastSimulationTime();

    TerminalWidget *mpTerminal;

signals:
    void startSimulation();
    void startProgressBarRefreshTimer(int ms);
    void done(bool);
};


#endif // SIMULATIONTHREADHANDLER_H
