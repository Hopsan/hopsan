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
#include "GUIObjects/GUIContainerObject.h"
#include "dcpmaster.h"
#include "dcpserver.h"
#include "GUIConnector.h"
#include "GUIPort.h"

namespace {

struct MetaTypeRegistrator
{
    MetaTypeRegistrator() {
        qRegisterMetaType<SimulationState>();
    }
};

}

static MetaTypeRegistrator gSimulationStateResistrator {};

void printRemoteCoreMessages(GUIMessageHandler *pMessageHandler, QVector<QString> &rTypes, QVector<QString> &rTags, QVector<QString> &rMessages)
{
    for (int i=0; i<rMessages.size(); ++i)
    {
       pMessageHandler->addMessageFromCore(rTypes[i], rTags[i], rMessages[i]);
    }
}

LocalSimulationWorkerObject::LocalSimulationWorkerObject(QVector<SystemObject *> vpSystems, const double startTime, const double stopTime, const double logStartTime, const unsigned int nLogSamples, const bool noChanges)
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
    emit setProgressState(SimulationState::Initialize);
    timer.start();
    initSuccess = simuHandler.initialize(mStartTime, mStopTime, mLogStartTime, mnLogSamples, coreSystemAccessVector);
    emit initDone(initSuccess, timer.elapsed());

    if (initSuccess)
    {
        // Simulating
        emit setProgressState(SimulationState::Simulate);

        // Check if we should simulate multiple systems at the same time using multicore
        if ((coreSystemAccessVector.size() > 1) && (gpConfig->getUseMulticore()))
        {
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gpConfig->getIntegerSetting(cfg::numberofthreads), coreSystemAccessVector, mNoChanges);
        }
        else if (gpConfig->getUseMulticore() && !gpConfig->getBoolSetting(cfg::logduringsimulation))
        {
            // Choose if we should simulate each system (or just the one system) using multiple cores (but each system in sequence)
            timer.start();
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gpConfig->getIntegerSetting(cfg::numberofthreads), coreSystemAccessVector, mNoChanges);
        }
        else if (gpConfig->getUseMulticore() && gpConfig->getBoolSetting(cfg::logduringsimulation))
        {
            // Choose if we should simulate each system (or just the one system) using multiple cores (but each system in sequence)
            int logSteps = gpConfig->getIntegerSetting(cfg::logsteps);
            timer.start();
            double time = mStartTime;
            simulateSuccess = true;
            bool noChanges = mNoChanges;
            for(int i=0; i<logSteps; ++i) {
                simulateSuccess = simulateSuccess && simuHandler.simulate(time, time+mStopTime/logSteps, -1, coreSystemAccessVector, noChanges);
                noChanges = false;
                time += mStopTime/logSteps;
                emit stepFinished();
            }
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, gpConfig->getIntegerSetting(cfg::numberofthreads), coreSystemAccessVector, mNoChanges);
        }
        else if(!gpConfig->getUseMulticore() && gpConfig->getBoolSetting(cfg::logduringsimulation))
        {
            int logSteps = gpConfig->getIntegerSetting(cfg::logsteps);
            timer.start();
            double time = mStartTime;
            simulateSuccess = true;
            for(int i=0; i<logSteps; ++i) {
                simulateSuccess = simulateSuccess && simuHandler.simulate(time, time+mStopTime/logSteps, -1, coreSystemAccessVector, mNoChanges);
                time += mStopTime/logSteps;
                emit stepFinished();
            }
        }
        else {
            timer.start();
            simulateSuccess = simuHandler.simulate(mStartTime, mStopTime, -1, coreSystemAccessVector, mNoChanges);
        }

        emit simulateDone(simulateSuccess, timer.elapsed());
    }

    // Finalizing
    emit setProgressState(SimulationState::Finalize);
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

    emit setProgressState(SimulationState::RemoteSimulate);
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

RemoteProgressbarWorkerObject::RemoteProgressbarWorkerObject(const double startTime, const double stopTime, SharedRemoteCoreSimulationHandlerT pRCSH)
{
    initialize(startTime, stopTime, QVector<SystemObject*>());
    mpRCSH = pRCSH;
    mProgress = 0;
}

#endif

void SimulationWorkerObjectBase::setMessageHandler(GUIMessageHandler *pMessageHandler)
{
     mpMessageHandler = pMessageHandler;
}

ProgressBarWorkerObject::ProgressBarWorkerObject() : QObject(nullptr)
{
    mpProgressDialogRefreshTimer = new QTimer(this);
    connect(mpProgressDialogRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshProgressBar()));
}

void ProgressBarWorkerObject::startRefreshTimer(int ts)
{
    // Start progress bar refresh timer with at least 50ms timestep (20 Hz), low values (close to 1) will freeze everything
    mpProgressDialogRefreshTimer->start(qMax(ts, 50));
}

void ProgressBarWorkerObject::stopRefreshTimer()
{
    mpProgressDialogRefreshTimer->stop();
}

void ProgressBarWorkerObject::setProgressBarState(SimulationState state)
{
    switch (state) {
    case SimulationState::Initialize:
        emit setProgressBarText(tr("Initializing..."));
        emit setProgressBarRange(0, 0);
        break;
    case SimulationState::Simulate:
        emit setProgressBarText(tr("Simulating..."));
        emit setProgressBarRange(0, 100);
        startRefreshTimer(gpConfig->getProgressBarStep());
        break;
    case SimulationState::RemoteSimulate:
        emit setProgressBarText(tr("Simulating..."));
        emit setProgressBarRange(0, 0);
        break;
    case SimulationState::DcpServerSimulate:
        emit setProgressBarText(tr("Running DCP simulation (server)..."));
        emit setProgressBarRange(0, 0);
        break;
    case SimulationState::DcpMasterSimulate:
        emit setProgressBarText(tr("Running DCP simulation (master)..."));
        emit setProgressBarRange(0, 100);
        startRefreshTimer(gpConfig->getProgressBarStep());
        break;
    case SimulationState::Finalize:
        stopRefreshTimer();
        emit setProgressBarText(tr("Finalizing..."));
        emit setProgressBarRange(0, 0);
        break;
    case SimulationState::Done:
        stopRefreshTimer();
        emit setProgressBarText(tr("Done"));
        emit setProgressBarRange(0, 0);
        break;
    }
}

void ProgressBarWorkerObject::initialize(const double startTime, const double stopTime, const QVector<SystemObject *> &rvSystems)
{
    mLastProgressRefreshStep = -1;
    mStartT = startTime;
    mStopT = stopTime;
    mvSystems = rvSystems;
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
            const int currentTimeStep = mpProgressDialogRefreshTimer->interval();
            mpProgressDialogRefreshTimer->setInterval( currentTimeStep + 10);
        }
    }
    qApp->processEvents();
}

void ProgressBarWorkerObject::abort()
{
    //! @todo how to handle abort for remote simulations
    for (int i=0; i<mvSystems.size(); ++i) {
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

void SimulationThreadHandler::initSimulateFinalize(SystemObject* pSystem, const bool noChanges)
{
    QVector<SystemObject*> vpSystems;
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

void SimulationThreadHandler::initSimulateFinalizeDcpMaster(SystemObject *pSystem, const QString &host, int port, bool realTime)
{
    mvpSystems.clear();
    mvpSystems.push_back(pSystem);
    mpSimulationWorkerObject = new DcpMasterSimulationWorkerObject(pSystem, host, port, mStartT, mStopT, realTime);
    mpSimulationWorkerObject->setMessageHandler(mpMessageHandler);
    initSimulateFinalizePrivate();
}


void SimulationThreadHandler::initSimulateFinalizeDcpServer(SystemObject* pSystem, const QString &host, int port, double communicationStep, const QString &targetFile)
{
    mvpSystems.clear();
    mvpSystems.push_back(pSystem);
    mpSimulationWorkerObject = new DcpServerSimulationWorkerObject(pSystem, host, port, communicationStep, targetFile);
    mpSimulationWorkerObject->setMessageHandler(mpMessageHandler);
    initSimulateFinalizePrivate();
}

void SimulationThreadHandler::initSimulateFinalize(QVector<SystemObject*> vpSystems, const bool noChanges)
{
    mvpSystems = vpSystems;
    mpSimulationWorkerObject = new LocalSimulationWorkerObject(mvpSystems, mStartT, mStopT, mLogStartTime, mnLogSamples, noChanges);
    initSimulateFinalizePrivate();
}

void SimulationThreadHandler::initSimulateFinalize_blocking(QVector<SystemObject*> vpSystems, const bool noChanges)
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

    if (gpConfig->getBoolSetting(cfg::progressbar) && mProgressBarEnabled)
    {
        // Create it if it does not exist (first time)
        if (!mpProgressDialog) {
            mpProgressDialog = new QProgressDialog(gpMainWindowWidget);
            mpProgressDialog->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
            mpProgressDialog->setWindowTitle("Running Simulation");
            // Increase size by 40% so that windows title will fit, could not figure out automatic size adjustment
            auto dialogSize = mpProgressDialog->size();
            dialogSize.rwidth() = dialogSize.width() + dialogSize.width()*0.4;
            mpProgressDialog->resize(dialogSize);
            mpProgressDialog->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
            mpProgressDialog->setMinimumWidth(dialogSize.width());

            mpProgressBarWorkerObject = new ProgressBarWorkerObject();
            mpProgressBarWorkerObject->moveToThread(&mProgressBarWorkerThread);

            connect(mpProgressBarWorkerObject, &ProgressBarWorkerObject::setProgressBarValue, mpProgressDialog, &QProgressDialog::setValue);
            connect(mpProgressBarWorkerObject, &ProgressBarWorkerObject::setProgressBarRange, mpProgressDialog, &QProgressDialog::setRange);
            connect(mpProgressBarWorkerObject, &ProgressBarWorkerObject::setProgressBarText,  mpProgressDialog, &QProgressDialog::setLabelText);

            connect(mpProgressBarWorkerObject, &ProgressBarWorkerObject::aborted, this, &SimulationThreadHandler::aborted);
        }


        if (mProgressBarModal) {
            mpProgressDialog->setWindowModality(Qt::WindowModal);
        }
        else {
            mpProgressDialog->setModal(false);
        }
        mpProgressDialog->show();

        // Initialize progress bar refresh worker object
        mpProgressBarWorkerObject->initialize(mStartT, mStopT, mvpSystems);

        // Start the progress bar worker thread and then signal the timer to start, so that it is started in the correct thread, will be problems otherwise
        mProgressBarWorkerThread.start(QThread::LowPriority);
    }

    // Create a timer to make sure messages are displayed in terminal during simulation
    // But not if we run multi threaded or a remote simulation
    if( (mpSimulationWorkerObject->swoType() != RemoteSWO) && !gpConfig->getUseMulticore())
    {
        if (mpCheckMessagesTimer == nullptr) {
            mpCheckMessagesTimer = new QTimer(this);
        }
        connect(mpSimulationWorkerObject, &SimulationWorkerObjectBase::simulateDone, mpCheckMessagesTimer, &QTimer::stop, Qt::BlockingQueuedConnection);
        connect(mpCheckMessagesTimer, &QTimer::timeout, mpMessageHandler, &GUIMessageHandler::collectHopsanCoreMessages);
        mpCheckMessagesTimer->setSingleShot(false);
        mpCheckMessagesTimer->start(1000);
    }

    // Connect simulation worker object signals
    if(gpConfig->getBoolSetting(cfg::logduringsimulation)) {
        connect(mpSimulationWorkerObject, SIGNAL(stepFinished()), this, SIGNAL(stepFinished()));
    }
    connect(this, &SimulationThreadHandler::startSimulation, mpSimulationWorkerObject, &SimulationWorkerObjectBase::initSimulateFinalize);
    connect(mpSimulationWorkerObject, &SimulationWorkerObjectBase::initDone, this, &SimulationThreadHandler::initDone);
    connect(mpSimulationWorkerObject, &SimulationWorkerObjectBase::simulateDone, this, &SimulationThreadHandler::simulateDone);
    connect(mpSimulationWorkerObject, &SimulationWorkerObjectBase::finalizeDone, this, &SimulationThreadHandler::finalizeDone);
    if (mpProgressBarWorkerObject) {
        connect(mpSimulationWorkerObject, &SimulationWorkerObjectBase::setProgressState, mpProgressBarWorkerObject, &ProgressBarWorkerObject::setProgressBarState);
    }
    if (mpProgressDialog) {
        connect(mpProgressDialog, &QProgressDialog::canceled, mpProgressBarWorkerObject, &ProgressBarWorkerObject::abort);
    }

    //! @todo make it possible to select priority in options
    // Start the simulation thread and then signal that the simulation can start
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

    // Disconnect abort signal from dialog, to avoid "abort button signal spamming"
    if (mpProgressDialog && mpProgressBarWorkerObject) {
        disconnect(mpProgressDialog, &QProgressDialog::canceled, mpProgressBarWorkerObject, &ProgressBarWorkerObject::abort);
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

    // Set progress status Done
    if (mpProgressBarWorkerObject) {
        mpProgressBarWorkerObject->setProgressBarState(SimulationState::Done);
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
    if (mpProgressDialog) {
        mpProgressDialog->close();
    }

    // Remove simulation worker object
    if (mpSimulationWorkerObject) {
        mpSimulationWorkerObject->deleteLater();
        mpSimulationWorkerObject = nullptr;
    }

    // Send done signal containing success or failed
    emit done(mInitSuccess && mSimuSucess && mFiniSucess && !mAborted);
}

void SimulationThreadHandler::aborted()
{
    mAborted = true;
}

SimulationThreadHandler::SimulationThreadHandler() :
    mpSimulationWorkerObject(nullptr), mpProgressDialog(nullptr), mpProgressBarWorkerObject(nullptr), mpCheckMessagesTimer(nullptr),
    mStartT(0), mStopT(1), mnLogSamples(0), mProgressBarEnabled(true), mProgressBarModal(true)
{
    mpMessageHandler = gpMessageHandler;
}

SimulationThreadHandler::~SimulationThreadHandler()
{
    if (mpProgressDialog) {
        mpProgressDialog->deleteLater();
    }
    if (mpProgressBarWorkerObject) {
        mpProgressBarWorkerObject->deleteLater();
    }
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

DcpServerSimulationWorkerObject::DcpServerSimulationWorkerObject(SystemObject *pSystem, const QString &host, int port, double communicationStep, const QString &targetFile)
{
    mpSystem = pSystem;
    mHost = host;
    mPort = port;
    mCommunicationStep = communicationStep;
    mTargetFile = targetFile;
}

void DcpServerSimulationWorkerObject::initSimulateFinalize()
{
    QTime timer;
    DcpServer *pDcpServer = new DcpServer(mpSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), mHost.toStdString(), mPort, mCommunicationStep, mpSystem->getNumberOfLogSamples());

    // Initializing
    emit setProgressState(SimulationState::Initialize);
    timer.start();
    pDcpServer->generateDcpFile(mTargetFile.toStdString());
    emit initDone(true, timer.elapsed());

    // Simulating
    emit setProgressState(SimulationState::DcpServerSimulate);
    gpMessageHandler->addInfoMessage("Starting a DCP simulation...");
    bool success = pDcpServer->start();
    gpMessageHandler->addInfoMessage("DCP simulation finished!");
    emit simulateDone(success, timer.elapsed());

    // Finalizing
    delete pDcpServer;
    // Finalizing
    emit setProgressState(SimulationState::Finalize);
    emit finalizeDone(true, timer.elapsed());
}

DcpMasterSimulationWorkerObject::DcpMasterSimulationWorkerObject(SystemObject *pSystem, const QString &host, int port, double startTime, double stopTime, bool realTime)
    : mpSystem(pSystem), mHost(host), mPort(port), mRealTime(realTime)
{
    mStartTime = startTime;
    mStopTime = stopTime;
}

void DcpMasterSimulationWorkerObject::initSimulateFinalize()
{
    // Initializing
    QTime timer;
    emit setProgressState(SimulationState::Initialize);
    timer.start();

    DcpMaster *pDcpMaster = new DcpMaster(mpSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), mHost.toStdString(), mPort, mpSystem->getTimeStep(), mStartTime, mStopTime, mRealTime);
    const QList<ModelObject *> modelObjects = mpSystem->getModelObjects();
    for(const auto comp : modelObjects) {
        if(comp->getTypeName() == HOPSANGUIDCPCOMPONENT) {   //Just in case, model shall only contain DCP components anyway
            pDcpMaster->addServer(comp->getParameterValue("dcpFile").toStdString());
        }
    }
    std::map<std::pair<size_t,size_t>,std::pair<std::vector<size_t>,std::vector<size_t> > > connections;
    const QList<Connector *> subConnectors = mpSystem->getSubConnectorPtrs();
    for(const auto &connection : subConnectors) {
        Port *pStartPort = connection->getStartPort();
        Port *pEndPort = connection->getEndPort();
        ModelObject *pStartComponent = pStartPort->getParentModelObject();
        ModelObject *pEndComponent = pEndPort->getParentModelObject();
        size_t fromServer = size_t(mpSystem->getModelObjects().indexOf(pStartComponent))+1;  //DCPLib uses one-based indexing
        size_t toServer = size_t(mpSystem->getModelObjects().indexOf(pEndComponent))+1;      //DCPLib uses one-based indexing
        QVector<CoreVariameterDescription> variameters;
        size_t fromVr, toVr;
        pStartComponent->getVariameterDescriptions(variameters);
        for(const auto &variameter : qAsConst(variameters)) {
            if(variameter.mPortName == pStartPort->getName()) {
                fromVr = variameter.mDescription.toUInt();
            }
        }
        pEndComponent->getVariameterDescriptions(variameters);
        for(const auto &variameter : qAsConst(variameters)) {
            if(variameter.mPortName == pEndPort->getName()) {
                toVr = variameter.mDescription.toUInt();
            }
        }

        if(connections.count(std::make_pair(fromServer,fromVr)) == 0) {
            connections[std::make_pair(fromServer,fromVr)] = std::make_pair(std::vector<size_t>(),std::vector<size_t>());
        }
        connections[std::make_pair(fromServer,fromVr)].first.push_back(toServer);
        connections[std::make_pair(fromServer,fromVr)].second.push_back(toVr);
    }

    for(const auto &con : connections) {
        pDcpMaster->addConnection(con.first.first, con.first.second, con.second.first, con.second.second);
    }

    emit initDone(true, timer.elapsed());

    // Simulating
    emit setProgressState(SimulationState::DcpMasterSimulate);
    gpMessageHandler->addInfoMessage("Starting a DCP simulation as master...");
    pDcpMaster->start();
    gpMessageHandler->addInfoMessage("DCP simulation finished!");
    emit simulateDone(true, timer.elapsed());

    // Finalizing
    delete pDcpMaster;
    // Finalizing
    emit setProgressState(SimulationState::Finalize);
    emit finalizeDone(true, timer.elapsed());
}
