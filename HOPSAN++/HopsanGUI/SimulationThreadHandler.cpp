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

#include "MainWindow.h"
#include "Widgets/HcomWidget.h"
#include "Configuration.h"
#include "common.h"
#include "GUIObjects/GUISystem.h"

void SimulationWorkerObject::initSimulateFinalize()
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
    initSuccess = simuHandler.initialize(mStartTime, mStopTime, mnLogSamples, coreSystemAccessVector);
    emit initDone(initSuccess, timer.elapsed());

    if (initSuccess)
    {
        // Simulating
        emit setProgressBarText(tr("Simulating..."));
        emit setProgressBarRange(0,100);

        // Check if we should simulate multiple systems at the same time using multicore
        if ((coreSystemAccessVector.size() > 1) && (gConfig.getUseMulticore()))
        {
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gConfig.getNumberOfThreads(), coreSystemAccessVector, mNoChanges);
        }
        else if (gConfig.getUseMulticore())
        {
            // Choose if we should simulate each system (or just the one system) using multiple cores (but each system in sequence)
            timer.start();
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gConfig.getNumberOfThreads(), coreSystemAccessVector, mNoChanges);
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

void SimulationWorkerObject::connectProgressDialog(QProgressDialog *pProgressDialog)
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

void ProgressBarWorkerObject::abort()
{
    for (int i=0; i<mvSystems.size(); ++i)
    {
        mvSystems[i]->getCoreSystemAccessPtr()->stop();
    }
    emit stopRefreshTimer();
    emit aborted();
}

void SimulationThreadHandler::setSimulationTimeVariables(const double startTime, const double stopTime, const unsigned int nLogSamples)
{
    mStartT = startTime;
    mStopT = stopTime;
    mnLogSamples = nLogSamples;
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

void SimulationThreadHandler::initSimulateFinalize(QVector<SystemContainer*> vpSystems, const bool noChanges)
{
    mInitSuccess=false; mSimuSucess=false; mFiniSucess=false; mAborted=false;

    mvpSystems = vpSystems;

    mpSimulationWorkerObject = new SimulationWorkerObject(mvpSystems, mStartT, mStopT, mnLogSamples, noChanges);
    mpSimulationWorkerObject->moveToThread(&mSimulationWorkerThread);

    connect(this, SIGNAL(startSimulation()), mpSimulationWorkerObject, SLOT(initSimulateFinalize()), Qt::UniqueConnection);
    connect(mpSimulationWorkerObject, SIGNAL(initDone(bool,int)), this, SLOT(initDone(bool,int)), Qt::UniqueConnection);
    connect(mpSimulationWorkerObject, SIGNAL(simulateDone(bool,int)), this, SLOT(simulateDone(bool,int)), Qt::UniqueConnection);
    connect(mpSimulationWorkerObject, SIGNAL(finalizeDone(bool,int)), this, SLOT(finalizeDone(bool,int)), Qt::UniqueConnection);

    if (gConfig.getEnableProgressBar() && mProgressBarEnabled)
    {
        mpProgressDialog = new QProgressDialog(gpMainWindow);
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

        // Start the progres bar worker thread and then signal the timer to start, so that it is started in the correct thread, will be problems otherwise
        mProgressBarWorkerThread.start(QThread::LowPriority);
        emit startProgressBarRefreshTimer(gConfig.getProgressBarStep());
    }

    //! @todo make it possible to select priority in options
    // Start the simulation thread and then signal that the simulation can start
    //simulationThread.start(QThread::TimeCriticalPriority);
    mSimulationWorkerThread.start(QThread::HighestPriority);
    emit startSimulation();
}

void SimulationThreadHandler::initSimulateFinalize_blocking(QVector<SystemContainer*> vpSystems, const bool noChanges)
{
    QEventLoop loop;
    connect(this, SIGNAL(done(bool)), &loop, SLOT(quit()), Qt::UniqueConnection);
    initSimulateFinalize(vpSystems, noChanges);
    loop.exec();
    //! @todo what happens if the simulation cmopletes before the loop is started, then it will never quit
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

    //! @todo maybe use signals and slots for messages instead
    gpMainWindow->mpTerminalWidget->checkMessages();

    // Handle printing of all the error messages
    if (mAborted)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(tr("Simulation was canceled by user"));
    }
    else if (!mInitSuccess)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(tr("Initialize was aborted due to some error"));
    }
    else if (!mSimuSucess)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(tr("Simulation was aborted due to some error"));
    }
    else if (!mFiniSucess)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(tr("Finalize was aborted due to some error"));
    }
    else
    {
        QString name;
        if (mvpSystems.size() > 1)
        {
            name = "MultipleModels";
        }
        else
        {
            name = mvpSystems[0]->getName();
        }
        QString msg = tr("Simulated").append(" '").append(name).append("' ").append(tr("successfully!"));
        msg.append(" ").append(tr("Initialization time: ")).append(QString::number(mInitTime).append(" ms"));
        msg.append(", ").append(tr("Simulation time: ").append(QString::number(mSimuTime)).append(" ms"));
        gpMainWindow->mpTerminalWidget->mpConsole->printInfoMessage(msg);
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

bool SimulationThreadHandler::wasSuccessful()
{
    return mInitSuccess && mSimuSucess && mFiniSucess && !mAborted;
}

int SimulationThreadHandler::getLastSimulationTime()
{
    return mSimuTime;
}
