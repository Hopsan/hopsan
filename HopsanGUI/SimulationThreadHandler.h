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
class GUIMessageHandler;

#include "CoreAccess.h"
#ifdef USEZMQ
#include "RemoteCoreAccess.h"
#endif


enum SimulationWorkeObjectEnumT {LocalSWO, RemoteSWO};

class SimulationWorkerObjectBase : public QObject
{
    Q_OBJECT

protected:
    double mStartTime;
    double mStopTime;
    double mLogStartTime;
    unsigned int mnLogSamples;
    bool mNoChanges;
    GUIMessageHandler *mpMessageHandler;

public:
    void setMessageHandler(GUIMessageHandler *pMessageHandler);
    void connectProgressDialog(QProgressDialog *pProgressDialog);
    virtual int swoType() const = 0;

public slots:
    virtual void initSimulateFinalize() = 0;

signals:
    void setProgressBarRange(int, int);
    void setProgressBarText(QString text);
    void closeProgressBarDialog();
    void initDone(bool, int);
    void simulateDone(bool, int);
    void finalizeDone(bool, int);
};

class LocalSimulationWorkerObject : public SimulationWorkerObjectBase
{
    Q_OBJECT
private:
    QVector<SystemContainer*> mvpSystems;
public:
    LocalSimulationWorkerObject(QVector<SystemContainer*> vpSystems, const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples, const bool noChanges);
    int swoType() const {return LocalSWO;}

public slots:
    void initSimulateFinalize();

};

#ifdef USEZMQ
class RemoteSimulationWorkerObject : public SimulationWorkerObjectBase
{
    Q_OBJECT
private:
    SharedRemoteCoreSimulationHandlerT mpRCSH;
    std::vector<std::string> *mpLogDataNames;
    std::vector<double> *mpLogData;
    double *mpProgress;
public:
    RemoteSimulationWorkerObject(SharedRemoteCoreSimulationHandlerT pRCSH, std::vector<std::string> *pLogNames, std::vector<double> *pLogData, double *pProgress,
                                 const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples);
    int swoType() const {return RemoteSWO;}

public slots:
    void initSimulateFinalize();
};
#endif

class ProgressBarWorkerObject : public QObject
{
    Q_OBJECT

private:
    QVector<SystemContainer*> mvSystems;

protected:
    QTimer mProgressDialogRefreshTimer;
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

#ifdef USEZMQ
class RemoteProgressbarWorkerObject : public ProgressBarWorkerObject
{
    Q_OBJECT
private:
    double mProgress;
    SharedRemoteCoreSimulationHandlerT mpRCSH;
public:
    RemoteProgressbarWorkerObject(const double startTime, const double stopTime, SharedRemoteCoreSimulationHandlerT pRCSH, QProgressDialog *pProgressDialog);
    //! @todo finnish this
};
#endif

class SimulationThreadHandler  : public QObject
{
    Q_OBJECT

private:
    QVector<SystemContainer*> mvpSystems;

    SimulationWorkerObjectBase *mpSimulationWorkerObject;
    ProgressBarWorkerObject *mpProgressBarWorkerObject;
    QProgressDialog *mpProgressDialog;
    GUIMessageHandler *mpMessageHandler;

    QThread mSimulationWorkerThread;
    QThread mProgressBarWorkerThread;

    double mStartT, mStopT, mLogStartTime;
    int mnLogSamples;

    bool mInitSuccess, mSimuSucess, mFiniSucess, mAborted, mProgressBarEnabled, mProgressBarModal;
    int mInitTime, mSimuTime, mFiniTime;

    void initSimulateFinalizePrivate();

protected slots:
    void initDone(bool success, int ms);
    void simulateDone(bool success, int ms);
    void finalizeDone(bool success, int ms);
    void aborted();

public:
    SimulationThreadHandler();

    void setSimulationTimeVariables(const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples);
    void setProgressDilaogBehaviour(bool enabled, bool modal);
    void initSimulateFinalize(SystemContainer* pSystem, const bool noChanges=false);
#ifdef USEZMQ
    void initSimulateFinalizeRemote(SharedRemoteCoreSimulationHandlerT pRCSH, std::vector<std::string> *pLogNames, std::vector<double> *pLogData, double *pProgress);
#endif
    void initSimulateFinalize(QVector<SystemContainer*> vpSystems, const bool noChanges=false);
    void initSimulateFinalize_blocking(QVector<SystemContainer*> vpSystems, const bool noChanges=false);
    bool wasSuccessful();
    int getLastSimulationTime();

    void setMessageHandler(GUIMessageHandler *pMessageHandler);

signals:
    void startSimulation();
    void startProgressBarRefreshTimer(int ms);
    void done(bool);
};


#endif // SIMULATIONTHREADHANDLER_H
