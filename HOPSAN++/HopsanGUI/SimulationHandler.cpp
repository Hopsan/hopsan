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
    initSuccess = mpSystem->getCoreSystemAccessPtr()->initialize(mStartTime, mStopTime, mnLogSamples);
    emit initDone(initSuccess, timer.elapsed());

    // Simulating
    emit setProgressBarText(tr("Simulating..."));
    emit setProgressBarRange(0,100);
    if(gConfig.getUseMulticore())
    {
        timer.start();
        mpSystem->getCoreSystemAccessPtr()->simulate(mStartTime, mStopTime, MULTICORE, gConfig.getNumberOfThreads());
    }
    else
    {
        timer.start();
        mpSystem->getCoreSystemAccessPtr()->simulate(mStartTime, mStopTime, SINGLECORE);
    }
    simulateSuccess = true; //! @todo need to check if sucess
    emit simulateDone(simulateSuccess, timer.elapsed());

    // Finalizing
    emit setProgressBarText(tr("Finalizing..."));
    emit setProgressBarRange(0,0);
    timer.start();
    mpSystem->getCoreSystemAccessPtr()->finalize();
    emit finalizeDone(true, timer.elapsed());
}

void SimulationObject::setProgressDialog(QProgressDialog * pProgressDialog)
{
    if (mpProgressDialog!=0)
    {
        //Disconnect from old progress bar
        disconnect(mpProgressDialog, 0);
    }

    mpProgressDialog = pProgressDialog;
    connect(this, SIGNAL(setProgressBarRange(int,int)), mpProgressDialog, SLOT(setRange(int,int)));
    connect(this, SIGNAL(setProgressBarText(QString)), mpProgressDialog, SLOT(setLabelText(QString)));
}

SimulationHandler::SimulationHandler(SystemContainer* pSystem, const double startTime, const double stopTime, const unsigned int nLogSamples)
    : mpSystem(pSystem), mStartT(startTime), mStopT(stopTime)
{
    mLastProgressRefreshStep = mStartT-1;
    mInitSuccess=false; mSimuSucess=false; mFiniSucess=false; mAborted=false;

    mpSimulationObject = new SimulationObject(mpSystem, mStartT, mStopT, nLogSamples);
    mpSimulationObject->moveToThread(&simulationThread);

    connect(this, SIGNAL(startSimulation()), mpSimulationObject, SLOT(doIt()));
    connect(mpSimulationObject, SIGNAL(initDone(bool,int)), this, SLOT(initDone(bool,int)));
    connect(mpSimulationObject, SIGNAL(simulateDone(bool,int)), this, SLOT(simulateDone(bool,int)));
    connect(mpSimulationObject, SIGNAL(finalizeDone(bool,int)), this, SLOT(finalizeDone(bool,int)));

    mProgressDialogRefreshTimer.setParent(this);
    mpProgressDialog = new QProgressDialog(gpMainWindow);
    mpProgressDialog->setWindowTitle("Running Simulation");
    mpProgressDialog->setModal(true);
    mpProgressDialog->show();

    mpSimulationObject->setProgressDialog(mpProgressDialog);

    connect(&mProgressDialogRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshProgressBar()));
    connect(this, SIGNAL(setProgressBarValue(int)), mpProgressDialog, SLOT(setValue(int)));
    connect(mpProgressDialog, SIGNAL(canceled()), this, SLOT(abort()));

    mProgressDialogRefreshTimer.start(gConfig.getProgressBarStep());
    simulationThread.start();
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
        QString msg = tr("Simulated").append(" '").append(mpSystem->getName()).append("' ").append(tr("successfully!"));
        msg.append(" ").append(tr("Initialization time: ")).append(QString::number(mInitTime).append(" ms"));
        msg.append(", ").append(tr("Simulation time: ").append(QString::number(mSimuTime)).append(" ms"));
        gpMainWindow->mpMessageWidget->printGUIInfoMessage(msg);
    }

    mpProgressDialog->close();
    simulationThread.terminate();
    simulationThread.wait();
    emit done(mInitSuccess && mSimuSucess && mFiniSucess && !mAborted);
}

void SimulationHandler::refreshProgressBar()
{
    const double t = mpSystem->getCoreSystemAccessPtr()->getCurrentTime();
    // Round up and truncate
    const int step = int((t-mStartT)/(mStopT - mStartT)*100.0+0.5);
    if (step > mLastProgressRefreshStep)
    {
        emit setProgressBarValue(step);
    }
}

void SimulationHandler::abort()
{
    mAborted = true;
    mpSystem->getCoreSystemAccessPtr()->stop();
}

bool SimulationHandler::wasSuccessful()
{
    return mInitSuccess && mSimuSucess && mFiniSucess && !mAborted;
}
