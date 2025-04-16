/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
class SystemObject;
class GUIMessageHandler;

#include "CoreAccess.h"
#ifdef USEZMQ
#include "RemoteCoreAccess.h"
#endif

enum SimulationWorkeObjectEnumT {LocalSWO, RemoteSWO, DCPMasterSWO, DcpServerSWO};
enum class SimulationState {Initialize, Simulate, RemoteSimulate, DcpMasterSimulate, DcpServerSimulate, Finalize, Done};

Q_DECLARE_METATYPE(SimulationState);

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
    virtual int swoType() const = 0;

public slots:
    virtual void initSimulateFinalize() = 0;

signals:
    void setProgressState(SimulationState);
    void initDone(bool, int);
    void simulateDone(bool, int);
    void stepFinished();
    void finalizeDone(bool, int);
};

class LocalSimulationWorkerObject : public SimulationWorkerObjectBase
{
    Q_OBJECT
private:
    QVector<SystemObject*> mvpSystems;
public:
    LocalSimulationWorkerObject(QVector<SystemObject*> vpSystems, const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples, const bool noChanges);
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
    QVector<RemoteResultVariable> *mpRemoteResultVariables;
    double *mpProgress;
public:
    RemoteSimulationWorkerObject(SharedRemoteCoreSimulationHandlerT pRCSH, QVector<RemoteResultVariable> *pRemoteResultVariables, double *pProgress,
                                 const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples);
    int swoType() const {return RemoteSWO;}

public slots:
    void initSimulateFinalize();
};
#endif

class DcpMasterSimulationWorkerObject : public SimulationWorkerObjectBase
{
    Q_OBJECT
private:
    SystemObject *mpSystem;
    QString mHost;
    int mPort;
    bool mRealTime;
public:
    DcpMasterSimulationWorkerObject(SystemObject *pSystem, const QString &host, int port, double startTime, double stopTime, bool realTime);
    int swoType() const {return DCPMasterSWO;}

public slots:
    void initSimulateFinalize();
};

class DcpServerSimulationWorkerObject : public SimulationWorkerObjectBase
{
    Q_OBJECT
private:
    SystemObject *mpSystem;
    QString mHost;
    int mPort;
    double mCommunicationStep;
    QString mTargetFile;
public:
    DcpServerSimulationWorkerObject(SystemObject *pSystem, const QString &host, int port, double communicationStep, const QString &targetFile);
    int swoType() const {return DcpServerSWO;}

public slots:
    void initSimulateFinalize();
};

class ProgressBarWorkerObject : public QObject
{
    Q_OBJECT

private:
    QVector<SystemObject*> mvSystems;
    QTimer *mpProgressDialogRefreshTimer;

protected:
    int mLastProgressRefreshStep;
    double mStartT, mStopT;

    void startRefreshTimer(int ts);
    void stopRefreshTimer();

protected slots:
    void refreshProgressBar();

public slots:
    void setProgressBarState(SimulationState state);
    void abort();

public:
    ProgressBarWorkerObject();
    void initialize(const double startTime, const double stopTime, const QVector<SystemObject*> &rvSystems);

signals:
    void setProgressBarRange(int, int);
    void setProgressBarText(QString text);
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
    RemoteProgressbarWorkerObject(const double startTime, const double stopTime, SharedRemoteCoreSimulationHandlerT pRCSH);
    //! @todo finnish this
};
#endif

class SimulationThreadHandler  : public QObject
{
    Q_OBJECT

private:
    QVector<SystemObject*> mvpSystems;

    SimulationWorkerObjectBase *mpSimulationWorkerObject;
    QProgressDialog *mpProgressDialog;
    ProgressBarWorkerObject *mpProgressBarWorkerObject;
    GUIMessageHandler *mpMessageHandler;
    QTimer *mpCheckMessagesTimer;

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
    ~SimulationThreadHandler();

    void setSimulationTimeVariables(const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples);
    void setProgressDilaogBehaviour(bool enabled, bool modal);
    void initSimulateFinalize(SystemObject* pSystem, const bool noChanges=false);
#ifdef USEZMQ
    void initSimulateFinalizeRemote(SharedRemoteCoreSimulationHandlerT pRCSH, QVector<RemoteResultVariable> *pRemoteResultVariables, double *pProgress);
#endif
    void initSimulateFinalizeDcpMaster(SystemObject *pSystem, const QString &host, int port, bool realTime);
    void initSimulateFinalizeDcpServer(SystemObject *pSystem, const QString &host, int port, double communicationStep, const QString &targetFile);
    void initSimulateFinalize(QVector<SystemObject*> vpSystems, const bool noChanges=false);
    void initSimulateFinalize_blocking(QVector<SystemObject*> vpSystems, const bool noChanges=false);
    bool wasSuccessful();
    int getLastSimulationTime();

    void setMessageHandler(GUIMessageHandler *pMessageHandler);

signals:
    void startSimulation();
    void stepFinished();
    void done(bool);
};


#endif // SIMULATIONTHREADHANDLER_H
