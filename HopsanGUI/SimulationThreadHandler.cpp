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

RemoteSimulationWorkerObject::RemoteSimulationWorkerObject(SharedRemoteCoreSimulationHandlerT pRCSH, std::vector<std::string> *pLogNames, std::vector<double> *pLogData, const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples)
{
    mpRCSH = pRCSH;
    mpLogDataNames = pLogNames;
    mpLogData = pLogData;

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
    bool simulateSuccess = mpRCSH->simulateModel();
    emit initDone(simulateSuccess, 0);
    emit simulateDone(simulateSuccess, timer.elapsed());

    // It is VERY important that we collect messages before we send finalizeDone signal as that will also do messaging and we could (will) have thread collission
    //! @todo since gpMessageHandler is indirectly used we could easily crash here if doing local simulation or just modelling at the same time
    QVector<QString> types,tags,messages;
    mpRCSH->getCoreMessages(types, tags, messages);
    printRemoteCoreMessages(mpMessageHandler, types, tags, messages);
    //! @todo should open a separate window with remote messages

    if (simulateSuccess)
    {
        // Collect data before emitting finalizeDone as that will signal that data is ready to be collected
        mpRCSH->getLogData(mpLogDataNames, mpLogData);

        emit finalizeDone(true, 0);
        mpRCSH.clear();
        return;
    }

    emit finalizeDone(false, 0);
    mpRCSH.clear();
}

#endif

void SimulationWorkerObjectBase::setMessageHandler(GUIMessageHandler *pMessageHandler)
{
     mpMessageHandler = pMessageHandler;
}

void SimulationWorkerObjectBase::connectProgressDialog(QProgressDialog *pProgressDialog)
{
    // Disconnect from old progress bar if any
    disconnect(this, SIGNAL(setProgressBarRange(int,int)), 0, 0);
    disconnect(this, SIGNAL(setProgressBarText(QString)), 0, 0);

    // Establish new connections
    connect(this, SIGNAL(setProgressBarRange(int,int)), pProgressDialog, SLOT(setRange(int,int)), Qt::UniqueConnection);
    connect(this, SIGNAL(setProgressBarText(QString)), pProgressDialog, SLOT(setLabelText(QString)), Qt::UniqueConnection);
}

ProgressBarWorkerObject::ProgressBarWorkerObject(const double startTime, const double stopTime, const QVector<SystemContainer*> &rvSystems, QProgressDialog *pProgressDialog)
{
    mLastProgressRefreshStep = -1;
    mStartT = startTime;
    mStopT = stopTime;
    mvSystems = rvSystems;

    mProgressDialogRefreshTimer.setParent(this);

    connect(pProgressDialog, SIGNAL(canceled()), this, SLOT(abort()), Qt::UniqueConnection);
    connect(this, SIGNAL(setProgressBarValue(int)), pProgressDialog, SLOT(setValue(int)), Qt::UniqueConnection);

    connect(&mProgressDialogRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshProgressBar()), Qt::UniqueConnection);
    connect(this, SIGNAL(stopRefreshTimer()), &mProgressDialogRefreshTimer, SLOT(stop()), Qt::UniqueConnection);
}

void ProgressBarWorkerObject::startRefreshTimer(int ts)
{
    // Start progress bar refresh timer with at least 50ms timestep (20 Hz), low values (close to 1) will freeze everything
    mProgressDialogRefreshTimer.start(std::max(ts, 50));
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
            int currentTimeStep = mProgressDialogRefreshTimer.interval();
            mProgressDialogRefreshTimer.setInterval( currentTimeStep + 10);
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
    emit stopRefreshTimer();
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
void SimulationThreadHandler::initSimulateFinalizeRemote(SharedRemoteCoreSimulationHandlerT pRCSH, std::vector<std::string> *pLogNames, std::vector<double> *pLogData)
{
    mvpSystems.clear();
    mpSimulationWorkerObject = new RemoteSimulationWorkerObject(pRCSH, pLogNames, pLogData, mStartT, mStopT, mLogStartTime, mnLogSamples);
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

    connect(this, SIGNAL(startSimulation()), mpSimulationWorkerObject, SLOT(initSimulateFinalize()), Qt::UniqueConnection);
    connect(mpSimulationWorkerObject, SIGNAL(initDone(bool,int)), this, SLOT(initDone(bool,int)), Qt::UniqueConnection);
    connect(mpSimulationWorkerObject, SIGNAL(simulateDone(bool,int)), this, SLOT(simulateDone(bool,int)), Qt::UniqueConnection);
    connect(mpSimulationWorkerObject, SIGNAL(finalizeDone(bool,int)), this, SLOT(finalizeDone(bool,int)), Qt::UniqueConnection);

    if (gpConfig->getBoolSetting(CFG_PROGRESSBAR) && mProgressBarEnabled)
    {
        mpProgressDialog = new QProgressDialog(gpMainWindowWidget);
        mpProgressDialog->setWindowTitle("Running Simulation");
        if (mProgressBarModal)
        {
            mpProgressDialog->setWindowModality(Qt::WindowModal);
        }
        else
        {
            mpProgressDialog->setModal(false);
        }
        mpProgressDialog->show();

        mpProgressBarWorkerObject = new ProgressBarWorkerObject(mStartT, mStopT, mvpSystems, mpProgressDialog); //!< @todo what about multiple systems
        connect(mpProgressDialog, SIGNAL(canceled()), mpProgressBarWorkerObject, SLOT(abort()), Qt::UniqueConnection);
        connect(mpProgressBarWorkerObject, SIGNAL(aborted()), this, SLOT(aborted()), Qt::UniqueConnection);
        connect(this, SIGNAL(startProgressBarRefreshTimer(int)), mpProgressBarWorkerObject, SLOT(startRefreshTimer(int)), Qt::UniqueConnection);
        connect(&mProgressBarWorkerThread, SIGNAL(finished()), mpProgressBarWorkerObject, SIGNAL(stopRefreshTimer()), Qt::UniqueConnection); //When thread finish we need to turn of timer

        // Move to worker thread and then connect the progress bar signals when both worker objects are in their respective threads
        mpProgressBarWorkerObject->moveToThread(&mProgressBarWorkerThread);
        mpSimulationWorkerObject->connectProgressDialog(mpProgressDialog);

        // Start the progress bar worker thread and then signal the timer to start, so that it is started in the correct thread, will be problems otherwise
        mProgressBarWorkerThread.start(QThread::LowPriority);
        emit startProgressBarRefreshTimer(gpConfig->getProgressBarStep());
    }

    // Create a timer to make sure messages are displayed in terminal during simulation
    // But not if we run multi threaded or a remote simulation
    if( (mpSimulationWorkerObject->swoType() != RemoteSWO) && !gpConfig->getUseMulticore())
    {
        QTimer *pCheckMessagesTimer = new QTimer();
        connect(pCheckMessagesTimer, SIGNAL(timeout()), mpMessageHandler, SLOT(collectHopsanCoreMessages()));
        connect(this, SIGNAL(done(bool)), pCheckMessagesTimer, SLOT(deleteLater()));
        pCheckMessagesTimer->setSingleShot(false);
        pCheckMessagesTimer->start(1000);
    }

    //! @todo make it possible to select priority in options
    // Start the simulation thread and then signal that the simulation can start
    //simulationThread.start(QThread::TimeCriticalPriority);
    mSimulationWorkerThread.start(QThread::HighestPriority);
    emit startSimulation();
}

void SimulationThreadHandler::initDone(bool success, int ms)
{
    mInitSuccess = success;
    mInitTime = ms;
}

void SimulationThreadHandler::simulateDone(bool success, int ms)
{
    mSimuSucess = success;
    mSimuTime = ms;
}

void SimulationThreadHandler::finalizeDone(bool success, int ms)
{
    mFiniSucess = success;
    mFiniTime = ms;

    // Request threads to shut down
    mSimulationWorkerThread.quit();
    mProgressBarWorkerThread.quit();

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

    // Wait until threads have been shut down before we delete objects
    mSimulationWorkerThread.wait();
    mProgressBarWorkerThread.wait();

    if (mpProgressBarWorkerObject != 0)
    {
        mpProgressBarWorkerObject->disconnect();
        mpProgressBarWorkerObject->deleteLater();
        mpProgressBarWorkerObject = 0;
    }

    if (mpProgressDialog != 0)
    {
        mpProgressDialog->disconnect();
        mpProgressDialog->close();
        mpProgressDialog->deleteLater();
        mpProgressDialog = 0;
    }

    if (mpSimulationWorkerObject != 0)
    {
        mpSimulationWorkerObject->disconnect();
        mpSimulationWorkerObject->deleteLater();
        mpSimulationWorkerObject = 0;
    }

    // Send done signal containing success or failed
    emit done(mInitSuccess && mSimuSucess && mFiniSucess && !mAborted);
}

void SimulationThreadHandler::aborted()
{
    mAborted = true;
}

SimulationThreadHandler::SimulationThreadHandler() :
    mpSimulationWorkerObject(0), mpProgressBarWorkerObject(0), mpProgressDialog(0), mStartT(0), mStopT(1), mnLogSamples(0), mProgressBarEnabled(true), mProgressBarModal(true)
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
