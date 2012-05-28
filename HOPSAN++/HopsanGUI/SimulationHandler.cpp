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
//! @file   SimulationHandler.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-25
//!
//! @brief Contains a class for running the simulation and signaling a progress bar
//!
//$Id$

#include "SimulationHandler.h"

#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include "Configuration.h"
#include "common.h"
#include "GUIObjects/GUISystem.h"

void SimulationObject::doIt()
{
    QTime timer;
    bool initSuccess, simulateSuccess;

    // Initializing
    emit setProgressBarText(tr("Initializing..."));
    emit setProgressBarRange(0,0);
    timer.start();
    for (int i=0; i<mvpSystems.size(); ++i)
    {
        initSuccess = initSuccess && mvpSystems[i]->getCoreSystemAccessPtr()->initialize(mStartTime, mStopTime, mnLogSamples);
    }
    emit initDone(initSuccess, timer.elapsed());

    // Simulating
    emit setProgressBarText(tr("Simulating..."));
    emit setProgressBarRange(0,100);

    // Check if we should simulate multiple systems at the same time using multicore
    if ((mvpSystems.size() > 1) && (gConfig.getUseMulticore()))
    {
        //Get core access from first one it will then handle the rest
        //! @todo this is strange need to fix in more clear way
        CoreSimulationHandler simuHandler;
        QVector<CoreSystemAccess*> vec;
        for (int i=0; i<mvpSystems.size(); ++i)
        {
            vec.push_back(mvpSystems[i]->getCoreSystemAccessPtr());
        }

        simuHandler.simulate(mStartTime, mStopTime, gConfig.getNumberOfThreads(), vec, mNoChanges);
        //mvpSystems.first()->getCoreSystemAccessPtr()->simulateAllOpenModels(mStartTime, mStopTime, MULTICORE, gConfig.getNumberOfThreads(), mNoChanges);
    }
    else
    {
        // Choose if we should simulate each system (or just the one system) using multiple cores (but each system in sequence)
        if(gConfig.getUseMulticore())
        {
            timer.start();
            for (int i=0; i<mvpSystems.size(); ++i)
            {
                mvpSystems[i]->getCoreSystemAccessPtr()->simulate(mStartTime, mStopTime, MULTICORE, gConfig.getNumberOfThreads());
            }
        }
        else
        {
            timer.start();
            for (int i=0; i<mvpSystems.size(); ++i)
            {
                mvpSystems[i]->getCoreSystemAccessPtr()->simulate(mStartTime, mStopTime, SINGLECORE);
            }
        }
    }

    simulateSuccess = true; //! @todo need to check if sucess
    emit simulateDone(simulateSuccess, timer.elapsed());

    // Finalizing
    emit setProgressBarText(tr("Finalizing..."));
    emit setProgressBarRange(0,0);
    timer.start();
    for (int i=0; i<mvpSystems.size(); ++i)
    {
        mvpSystems[i]->getCoreSystemAccessPtr()->finalize();
    }
    emit finalizeDone(true, timer.elapsed());
}

void SimulationObject::setProgressDialog(QProgressDialog * pProgressDialog)
{
    // Disconnect from old progress bar if any
    disconnect(this, SIGNAL(setProgressBarRange(int,int)), 0, 0);
    disconnect(this, SIGNAL(setProgressBarText(QString)), 0, 0);

    // Establish new connections
    connect(this, SIGNAL(setProgressBarRange(int,int)), pProgressDialog, SLOT(setRange(int,int)));
    connect(this, SIGNAL(setProgressBarText(QString)), pProgressDialog, SLOT(setLabelText(QString)));
}

void SimulationHandler::simulate(QVector<SystemContainer*> vpSystems, const double startTime, const double stopTime, const unsigned int nLogSamples, const bool noChanges)
{
    mLastProgressRefreshStep = mStartT-1;
    mInitSuccess=false; mSimuSucess=false; mFiniSucess=false; mAborted=false;

    mvpSystems = vpSystems;
    mStartT = startTime;
    mStopT = stopTime;

    if (mpSimulationObject != 0)
    {
        delete mpSimulationObject;
    }
    mpSimulationObject = new SimulationObject(mvpSystems, mStartT, mStopT, nLogSamples, noChanges);
    mpSimulationObject->moveToThread(&simulationThread);

    connect(this, SIGNAL(startSimulation()), mpSimulationObject, SLOT(doIt()));
    connect(mpSimulationObject, SIGNAL(initDone(bool,int)), this, SLOT(initDone(bool,int)));
    connect(mpSimulationObject, SIGNAL(simulateDone(bool,int)), this, SLOT(simulateDone(bool,int)));
    connect(mpSimulationObject, SIGNAL(finalizeDone(bool,int)), this, SLOT(finalizeDone(bool,int)));

    if (gConfig.getEnableProgressBar())
    {
        if (mpProgressDialog == 0)
        {
            //! @todo Cant this be an ordinary member
            mpProgressDialog = new QProgressDialog(gpMainWindow);
            connect(this, SIGNAL(setProgressBarValue(int)), mpProgressDialog, SLOT(setValue(int)));
            connect(mpProgressDialog, SIGNAL(canceled()), this, SLOT(abort()));
        }
        mpProgressDialog->setWindowTitle("Running Simulation");
        mpProgressDialog->setModal(false);
        mpProgressDialog->show();

        mpSimulationObject->setProgressDialog(mpProgressDialog);

        mProgressDialogRefreshTimer.setParent(this);
        connect(&mProgressDialogRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshProgressBar()), Qt::UniqueConnection);

        mProgressDialogRefreshTimer.start(gConfig.getProgressBarStep());
    }

    //! @todo make it possible to select priority in options
    //simulationThread.start(QThread::TimeCriticalPriority);
    simulationThread.start(QThread::HighestPriority);
    //simulationThread.start(QThread::HighPriority);
    emit startSimulation();
}

void SimulationHandler::initDone(bool success, int ms)
{
    mInitSuccess = success;
    mInitTime = ms;
}

void SimulationHandler::simulateDone(bool success, int ms)
{
    mSimuSucess = success;
    mSimuTime = ms;
}

void SimulationHandler::finalizeDone(bool success, int ms)
{
    mFiniSucess = success;
    mFiniTime = ms;

    //! @todo maybe use signals and slots for messages instead
    gpMainWindow->mpMessageWidget->checkMessages();

    // Handle printing of all the error messages
    if (mAborted)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage(tr("Simulation was canceled by user"));
    }
    else if (!mInitSuccess)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage(tr("Initialize was aborted due to some error"));
    }
    else if (!mSimuSucess)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage(tr("Simulation was aborted due to some error"));
    }
    else if (!mFiniSucess)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage(tr("Finalize was aborted due to some error"));
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
        gpMainWindow->mpMessageWidget->printGUIInfoMessage(msg);
    }

    if (mpProgressDialog != 0)
    {
        mpProgressDialog->close();
        mpProgressDialog->deleteLater();
        mpProgressDialog = 0;
    }

    simulationThread.quit();
    simulationThread.wait();
    emit done(mInitSuccess && mSimuSucess && mFiniSucess && !mAborted);
}

void SimulationHandler::refreshProgressBar()
{
    //! @todo this will give incorrect update for multi system simulations
    const double t = mvpSystems[0]->getCoreSystemAccessPtr()->getCurrentTime();
    // Round up and truncate
    const int step = int((t-mStartT)/(mStopT - mStartT)*100.0+0.5);
    if (step > mLastProgressRefreshStep)
    {
        mLastProgressRefreshStep = step;
        emit setProgressBarValue(step);
    }
}

void SimulationHandler::abort()
{
    mAborted = true;
    for (int i=0; i<mvpSystems.size(); ++i)
    {
        mvpSystems[i]->getCoreSystemAccessPtr()->stop();
    }
}

bool SimulationHandler::wasSuccessful()
{
    return mInitSuccess && mSimuSucess && mFiniSucess && !mAborted;
}
