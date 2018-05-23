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
//! @file   SimulationThreadHandler.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-25
//!
//! @brief Contains a class for running the simulation and signaling a progress bar
//!
//$Id$

#include "SimulationThreadHandler.h"

#include "MessageHandler.h"
#include "Configuration.h"
#include "common.h"
#include "global.h"
#include "GUIObjects/GUISystem.h"

void printRemoteCoreMessages(GUIMessageHandler *pMessageHandler, QVector<QString> &rTypes, QVector<QString> &rTags, QVector<QString> &rMessages)
{
    for (int i=0; i<rMessages.size(); ++i)
    {
       pMessageHandler->addMessageFromCore(rTypes[i], rTags[i], rMessages[i]);
    }
}

LocalSimulationWorkerObject::LocalSimulationWorkerObject(QVector<SystemContainer *> vpSystems, const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples, const bool noChanges)
{
    mvpSystems = vpSystems;
    mStartTime = startTime;
    mStopTime = stopTime;
    mLogStartTime = logStartTime;
    mnLogSamples = nLogSamples;
    mNoChanges = noChanges;
    mpMessageHandler = gpMessageHandler;
}

void LocalSimulationWorkerObject::initSimulateFinalize()
{
    QTime timer;
    bool initSuccess, simulateSuccess;
    CoreSimulationHandler simuHandler;
    QVector<CoreSystemAccess*> coreSystemAccessVector;
    for (int i=0; i<mvpSystems.size(); ++i)
    {
        coreSystemAccessVector.push_back(mvpSystems[i]->getCoreSystemAccessPtr());
    }

    // Initializing
    emit setProgressBarText(tr("Initializing..."));
    emit setProgressBarRange(0,0);
    timer.start();
    initSuccess = simuHandler.initialize(mStartTime, mStopTime, mLogStartTime, mnLogSamples, coreSystemAccessVector);
    emit initDone(initSuccess, timer.elapsed());

    if (initSuccess)
    {
        // Simulating
        emit setProgressBarText(tr("Simulating..."));
        emit setProgressBarRange(0,100);

        // Check if we should simulate multiple systems at the same time using multicore
        if ((coreSystemAccessVector.size() > 1) && (gpConfig->getUseMulticore()))
        {
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS), coreSystemAccessVector, mNoChanges);
        }
        else if (gpConfig->getUseMulticore())
        {
            // Choose if we should simulate each system (or just the one system) using multiple cores (but each system in sequence)
            timer.start();
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS), coreSystemAccessVector, mNoChanges);
        }
        else
        {
            timer.start();
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, -1, coreSystemAccessVector, mNoChanges);
        }

        emit simulateDone(simulateSuccess, timer.elapsed());
    }

    // Finalizing
    emit setProgressBarText(tr("Finalizing..."));
    emit setProgressBarRange(0,0);
    timer.start();
    simuHandler.finalize(coreSystemAccessVector);
    emit finalizeDone(true, timer.elapsed());
}

#ifdef USEZMQ

RemoteSimulationWorkerObject::RemoteSimulationWorkerObject(SharedRemoteCoreSimulationHandlerT pRCSH, QVector<RemoteResultVariable> *pRemoteResultVariables, double *pProgress,
                                                           const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples)
{
    mpRCSH = pRCSH;
    mpRemoteResultVariables = pRemoteResultVariables;
    mpProgress = pProgress;

    mStartTime = startTime;
    mStopTime = stopTime;
    mLogStartTime = logStartTime;
    mnLogSamples = nLogSamples;
    mNoChanges = false;
}

void RemoteSimulationWorkerObject::initSimulateFinalize()
{
    QTime timer;

    emit setProgressBarText(tr("Simulating..."));
    timer.start();
    bool simulateSuccess = mpRCSH->simulateModel_blocking(mpProgress);
    emit initDone(simulateSuccess, 0);
    emit simulateDone(simulateSuccess, timer.elapsed());

    // It is VERY important that we collect messages before we send finalizeDone signal as that will also do messaging and we could (will) have thread collission
    QVector<QString> types,tags,messages;
    bool gotMessages = mpRCSH->getCoreMessages(types, tags, messages);
    printRemoteCoreMessages(mpMessageHandler, types, tags, messages);
    //! @todo should open a separate window with remote messages

    // Collect data before emitting finalizeDone as that will signal that data is ready to be collected
    bool gotLogData = mpRCSH->getLogData(*mpRemoteResultVariables);
    if (!gotLogData)
    {
        mpMessageHandler->addWarningMessage("Failed to get remote results");
    }

    if (simulateSuccess && gotMessages && gotLogData)
    {
        emit finalizeDone(true, 0);
        mpRCSH.clear();
        return;
    }

    emit finalizeDone(false, 0);
    mpRCSH.clear();
}

RemoteProgressbarWorkerObject::RemoteProgressbarWorkerObject(const double startTime, const double stopTime, SharedRemoteCoreSimulationHandlerT pRCSH) :
    ProgressBarWorkerObject(startTime, stopTime, QVector<SystemContainer*>())
{
    mpRCSH = pRCSH;
    mProgress = 0;
}

#endif

void SimulationWorkerObjectBase::setMessageHandler(GUIMessageHandler *pMessageHandler)
{
     mpMessageHandler = pMessageHandler;
}

ProgressBarWorkerObject::ProgressBarWorkerObject(const double startTime, const double stopTime, const QVector<SystemContainer*> &rvSystems)
{
    mLastProgressRefreshStep = -1;
    mStartT = startTime;
    mStopT = stopTime;
    mvSystems = rvSystems;

    mpProgressDialogRefreshTimer = new QTimer(this); //Note! delete is handled automatically since this is parent
    connect(mpProgressDialogRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshProgressBar()));
}

void ProgressBarWorkerObject::startRefreshTimer(int ts)
{
    // Start progress bar refresh timer with at least 50ms timestep (20 Hz), low values (close to 1) will freeze everything
    qDebug() << "Starting refresh timer";
    mpProgressDialogRefreshTimer->start(qMax(ts, 50));
}

void ProgressBarWorkerObject::stopRefreshTimer()
{
    qDebug() << "Stopping refresh timer";
    mpProgressDialogRefreshTimer->stop();
}

void ProgressBarWorkerObject::refreshProgressBar()
{
    if (!mvSystems.isEmpty())
    {
        //! @todo this will give incorrect update for multi system simulations
        const double t = mvSystems[0]->getCoreSystemAccessPtr()->getCurrentTime();

        // Round up and truncate
        const int step = int((t-mStartT)/(mStopT - mStartT)*100.0 + 0.5);
        if( step > mLastProgressRefreshStep)
        {
            mLastProgressRefreshStep = step;
            emit setProgressBarValue(step);
        }
        else
        {
            // adapt timer timestep to the simulation model
            int currentTimeStep = mpProgressDialogRefreshTimer->interval();
            mpProgressDialogRefreshTimer->setInterval( currentTimeStep + 10);
        }
    }
}

void ProgressBarWorkerObject::abort()
{
    //! @todo how to handle abort for remote simulations
    for (int i=0; i<mvSystems.size(); ++i)
    {
        mvSystems[i]->getCoreSystemAccessPtr()->stop();
    }
    stopRefreshTimer();
    emit aborted();
}

void SimulationThreadHandler::setSimulationTimeVariables(const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples)
{
    mStartT = startTime;
    mStopT = stopTime;
    mnLogSamples = nLogSamples;
    mLogStartTime = logStartTime;
}

void SimulationThreadHandler::setProgressDilaogBehaviour(bool enabled, bool modal)
{
    mProgressBarEnabled = enabled;
    mProgressBarModal = modal;
}

void SimulationThreadHandler::initSimulateFinalize(SystemContainer* pSystem, const bool noChanges)
{
    QVector<SystemContainer*> vpSystems;
    vpSystems.push_back(pSystem);
    initSimulateFinalize(vpSystems, noChanges);
}

#ifdef USEZMQ
void SimulationThreadHandler::initSimulateFinalizeRemote(SharedRemoteCoreSimulationHandlerT pRCSH, QVector<RemoteResultVariable> *pRemoteResultVariables, double *pProgress)
{
    mvpSystems.clear();
    mpSimulationWorkerObject = new RemoteSimulationWorkerObject(pRCSH, pRemoteResultVariables, pProgress, mStartT, mStopT, mLogStartTime, mnLogSamples);
    mpSimulationWorkerObject->setMessageHandler(mpMessageHandler);
    initSimulateFinalizePrivate();
}
#endif

void SimulationThreadHandler::initSimulateFinalize(QVector<SystemContainer*> vpSystems, const bool noChanges)
{
    mvpSystems = vpSystems;
    mpSimulationWorkerObject = new LocalSimulationWorkerObject(mvpSystems, mStartT, mStopT, mLogStartTime, mnLogSamples, noChanges);
    initSimulateFinalizePrivate();
}

void SimulationThreadHandler::initSimulateFinalize_blocking(QVector<SystemContainer*> vpSystems, const bool noChanges)
{
    QEventLoop loop;
    connect(this, SIGNAL(done(bool)), &loop, SLOT(quit()), Qt::UniqueConnection);
    initSimulateFinalize(vpSystems, noChanges);
    loop.exec();
    //! @todo what happens if the simulation completes before the loop is started, then it will never quit
}

void SimulationThreadHandler::initSimulateFinalizePrivate()
{
    mInitSuccess=false; mSimuSucess=false; mFiniSucess=false; mAborted=false;
    mpSimulationWorkerObject->moveToThread(&mSimulationWorkerThread);
    connect(&mSimulationWorkerThread, SIGNAL(finished()), mpSimulationWorkerObject, SLOT(deleteLater()));

    if (gpConfig->getBoolSetting(CFG_PROGRESSBAR) && mProgressBarEnabled)
    {
        // Create it if it does not exist (first time)
        if (!mpProgressDialog)
        {
            //mpProgressDialog = new QProgressDialog(gpMainWindowWidget);
            mpProgressDialog = new MyProgressDialog(gpMainWindowWidget);
        }
        mpProgressDialog->setWindowTitle("Running Simulation");
        if (mProgressBarModal)
        {
            mpProgressDialog->setWindowModality(Qt::WindowModal);
        }
        else
        {
            mpProgressDialog->setModal(false);
        }
        qDebug() << "showing progress dialog";
        mpProgressDialog->show();

        // Create a new progress update worker object
        ProgressBarWorkerObject *pProgressBarWorkerObject = new ProgressBarWorkerObject(mStartT, mStopT, mvpSystems); //!< @todo what about multiple systems
        pProgressBarWorkerObject->moveToThread(&mProgressBarWorkerThread);
        connect(&mProgressBarWorkerThread, SIGNAL(finished()), pProgressBarWorkerObject, SLOT(deleteLater()));

        // Connect signals between progress update worker object and progress dialog
        connect(mpProgressDialog, SIGNAL(canceled()), pProgressBarWorkerObject, SLOT(abort()));
        connect(pProgressBarWorkerObject, SIGNAL(setProgressBarValue(int)), mpProgressDialog, SLOT(setValue(int)), Qt::BlockingQueuedConnection);

        // Connect signals between progress update worker object and this thread handler
        connect(pProgressBarWorkerObject, SIGNAL(aborted()), this, SLOT(aborted()));
        connect(this, SIGNAL(startProgressBarRefreshTimer(int)), pProgressBarWorkerObject, SLOT(startRefreshTimer(int)), Qt::BlockingQueuedConnection);
        connect(this, SIGNAL(stopProgressBarRefreshTimer()), pProgressBarWorkerObject, SLOT(stopRefreshTimer()), Qt::BlockingQueuedConnection);

        // Connect signals between simulation worker object and the progress dialog
        connect(mpSimulationWorkerObject, SIGNAL(setProgressBarRange(int,int)), mpProgressDialog, SLOT(setRange(int,int)), Qt::BlockingQueuedConnection);
        connect(mpSimulationWorkerObject, SIGNAL(setProgressBarText(QString)), mpProgressDialog, SLOT(setLabelText(QString)), Qt::BlockingQueuedConnection);

        // Start the progress bar worker thread and then signal the timer to start, so that it is started in the correct thread, will be problems otherwise
        mProgressBarWorkerThread.start(QThread::LowPriority);
    }

    // Create a timer to make sure messages are displayed in terminal during simulation
    // But not if we run multi threaded or a remote simulation
    if( (mpSimulationWorkerObject->swoType() != RemoteSWO) && !gpConfig->getUseMulticore())
    {
        QTimer *pCheckMessagesTimer = new QTimer(this);
        connect(pCheckMessagesTimer, SIGNAL(timeout()), mpMessageHandler, SLOT(collectHopsanCoreMessages()));
        // The following connections should stop and delete the time when the simulation has finished
        connect(mpSimulationWorkerObject, SIGNAL(simulateDone(bool,int)), pCheckMessagesTimer, SLOT(stop()), Qt::BlockingQueuedConnection);
        connect(mpSimulationWorkerObject, SIGNAL(simulateDone(bool,int)), pCheckMessagesTimer, SLOT(deleteLater()));
        pCheckMessagesTimer->setSingleShot(false);
        pCheckMessagesTimer->start(1000);
    }

    // Connect simulation worker object signals
    connect(this, SIGNAL(startSimulation()), mpSimulationWorkerObject, SLOT(initSimulateFinalize()));
    connect(mpSimulationWorkerObject, SIGNAL(initDone(bool,int)), this, SLOT(initDone(bool,int)), Qt::BlockingQueuedConnection);
    connect(mpSimulationWorkerObject, SIGNAL(simulateDone(bool,int)), this, SLOT(simulateDone(bool,int)), Qt::BlockingQueuedConnection);
    connect(mpSimulationWorkerObject, SIGNAL(finalizeDone(bool,int)), this, SLOT(finalizeDone(bool,int)));

    //! @todo make it possible to select priority in options
    // Start the simulation thread and then signal that the simulation can start
    mSimulationWorkerThread.start(QThread::HighestPriority);
    emit startSimulation();
}

void SimulationThreadHandler::initDone(bool success, int ms)
{
    mInitSuccess = success;
    mInitTime = ms;
    if (mInitSuccess) {
        qDebug() << "Emitting start progress bar refresh";
        emit startProgressBarRefreshTimer(gpConfig->getProgressBarStep());
        qDebug() << "After Emitting start progress bar refresh";
    }
}

void SimulationThreadHandler::simulateDone(bool success, int ms)
{
    mSimuSucess = success;
    mSimuTime = ms;
    emit stopProgressBarRefreshTimer();
}

void SimulationThreadHandler::finalizeDone(bool success, int ms)
{
    mFiniSucess = success;
    mFiniTime = ms;

    // Disconnect signals from dialog, to avoid "abort button signal spaming"
    if (mpProgressDialog)
    {
        mpProgressDialog->disconnect();
    }

    // Collect core messages prior to showing Simulation finished message
    mpMessageHandler->collectHopsanCoreMessages();

    // Handle printing of all the error messages
    if (mAborted)
    {
        mpMessageHandler->addErrorMessage(tr("Simulation was canceled by user"));
    }
    else if (!mInitSuccess)
    {
        mpMessageHandler->addErrorMessage(tr("Initialize was stopped or aborted for some reason"));
    }
    else if (!mSimuSucess)
    {
        mpMessageHandler->addErrorMessage(tr("Simulation was stopped or aborted for some reason"));
    }
    else if (!mFiniSucess)
    {
        mpMessageHandler->addErrorMessage(tr("Finalize was stopped or aborted for some reason"));
    }
    else
    {
        QString name;
        if (mvpSystems.size() > 1)
        {
            name = "MultipleModels";
        }
        else if (!mvpSystems.isEmpty())
        {
            name = mvpSystems[0]->getName();
        }
        QString msg = tr("Simulated").append(" '").append(name).append("' ").append(tr("successfully!"));
        msg.append(" ").append(tr("Initialization time: ")).append(QString::number(mInitTime).append(" ms"));
        msg.append(", ").append(tr("Simulation time: ").append(QString::number(mSimuTime)).append(" ms"));
        mpMessageHandler->addInfoMessage(msg);
    }

    // Forget the worker object (it should be auto deleted)
    if (mpSimulationWorkerObject)
    {
        mpSimulationWorkerObject = nullptr;
    }

    // Request threads to shut down
    mSimulationWorkerThread.quit();
    mProgressBarWorkerThread.quit();

    // Just to be safe, process any events in this thread, (like signal queues to the mpProgressDialog)
    QEventLoop loop;
    loop.processEvents();

    // Wait until threads have been shut down
    mSimulationWorkerThread.wait();
    mProgressBarWorkerThread.wait();

    // Close the progress dialog
    if (mpProgressDialog)
    {
        qDebug() << "closing progress dialog";
        mpProgressDialog->close();
        mpProgressDialog->deleteLater();
        mpProgressDialog = nullptr;
    }

    // Send done signal containing success or failed
    emit done(mInitSuccess && mSimuSucess && mFiniSucess && !mAborted);
}

void SimulationThreadHandler::aborted()
{
    mAborted = true;
}

SimulationThreadHandler::SimulationThreadHandler() :
    mpSimulationWorkerObject(nullptr), mpProgressDialog(nullptr), mStartT(0), mStopT(1), mnLogSamples(0), mProgressBarEnabled(true), mProgressBarModal(true)
{
    mpMessageHandler = gpMessageHandler;
}

bool SimulationThreadHandler::wasSuccessful()
{
    return mInitSuccess && mSimuSucess && mFiniSucess && !mAborted;
}

int SimulationThreadHandler::getLastSimulationTime()
{
    return mSimuTime;
}

void SimulationThreadHandler::setMessageHandler(GUIMessageHandler *pMessageHandler)
{
    mpMessageHandler = pMessageHandler;
}
