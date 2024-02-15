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

//$Id$

#include "RemoteSimulationUtils.h"
#include "Configuration.h"
#include "global.h"
#include "Widgets/ModelWidget.h"
#include "MessageHandler.h"

#ifdef USEZMQ

#include <QMutex>
#include <QSemaphore>
#include <QObject>
#include <QQueue>
#include <QEventLoop>
#include <QDebug>
#include <QTime>
#include <chrono>
#include <cmath>

using namespace std::chrono;


class MyBarrier : public QObject
{
    Q_OBJECT
public:
    bool tryLock()
    {
        return mMutex.tryLock();
    }

public slots:
    void unlock()
    {
        mMutex.unlock();
    }

private:
    QMutex mMutex;
};

class MySemaphore : public QObject, public QSemaphore
{
    Q_OBJECT
public:
    MySemaphore(int n) : QObject(0), QSemaphore(n)  {}

public slots:
    void releaseSlot()
    {
        this->release();
    }
};

class MyProgressTracker
{
public:
    MyProgressTracker(int historyLength=1)
    {
        initialize(historyLength);
    }

    void initialize(int historyLength)
    {
        mHasAddedFirstTime=false;
        mFirstAddTime = steady_clock::time_point();
        mLastAddTime = steady_clock::time_point();
        mTimes = QVector<steady_clock::time_point>(qMax(historyLength,1), steady_clock::now());
        mProgress = QVector<double>(qMax(historyLength,1), 0.0);
        mLastCalculatedRemainingTime=-1;
        mLastEstimatedTotalSimtime=-1;
    }

    double secondsSinceLastAdd() const
    {
        return duration_cast<seconds>(steady_clock::now()-mLastAddTime).count();
    }

    void addProgress(const double progress)
    {
        mProgress.push_back(progress);
        mTimes.push_back(steady_clock::now());
        mProgress.pop_front();
        mTimes.pop_front();
        mLastAddTime = steady_clock::now();
        //! @todo circle buffer would be better
        if (!mHasAddedFirstTime)
        {
            mFirstAddTime = steady_clock::now();
            mHasAddedFirstTime=true;
        }
    }

    double estimateRemainingTime()
    {
        // yi = k*xi+m, y=progress, m=oldestprogress, x=time relative oldest time
        // => estimate k from samples using least squares method
        // Yi = yi-m
        // sum ( Yi - k*xi)^2 -> 0
        // => sum ( Yi^2 + (kxi)^2 - 2(Yi*k*xi) ) = 0 .... hmm get code from Wikipedia instead

//        QVector<double> vx,vy;
//        vx.resize(mTimes.size());
//        vy.resize(mTimes.size());
        double sumxi=0, sumyi=0, sumxiyi=0, sumxi2=0;
        double n = mTimes.size();
        for (int i=0; i<mTimes.size(); ++i)
        {
            const double xi = duration_cast<seconds>(mTimes[i]-mTimes[0]).count();
            const double yi = mProgress[i]-mProgress[0];
//            vx[i] = xi;
//            vy[i] = yi;
            sumxi += xi;
            sumyi += yi;
            sumxi2 += xi*xi;
            sumxiyi += xi*yi;
        }
        double k_est = (sumxiyi-(sumxi*sumyi)/n)/(sumxi2-(sumxi*sumxi)/n);

//        if (mProgress[0] > 0)
//        {
//            gpMessageHandler->addInfoMessage(QString("%1").arg(k_est));
//        }


        // Now figure out estimated remaining time
        // y = 1-m = k_est * dX => dX = (1-m)/k_est;

        if (k_est > 0)
        {
            mLastCalculatedRemainingTime = (1.0-mProgress.last())/k_est;
            double progressedTime = duration_cast<seconds>(steady_clock::now() - mFirstAddTime).count();
            mLastEstimatedTotalSimtime = progressedTime+mLastCalculatedRemainingTime;
        }
        return mLastCalculatedRemainingTime;
    }

    double lastCalculatedRemainingTime() const
    {
        return mLastCalculatedRemainingTime;
    }

    double lastEstimatedTotalSimtime() const
    {
        return mLastEstimatedTotalSimtime;
    }

    double lastTrackedProgress() const
    {
        return mProgress.last();
    }

    int length() const
    {
        return mProgress.size();
    }


private:
    bool mHasAddedFirstTime;
    steady_clock::time_point mFirstAddTime;
    steady_clock::time_point mLastAddTime;
    QVector<steady_clock::time_point> mTimes;
    QVector<double> mProgress;
    double mLastCalculatedRemainingTime;
    double mLastEstimatedTotalSimtime;
};

RemoteSimulationQueueHandler::~RemoteSimulationQueueHandler()
{
    clear();
}

void RemoteSimulationQueueHandler::setupModelQueues(QVector<ModelWidget *> models, int numThreads)
{
    reset();
    mAllModels = models;

    bool addrserver_connected = connectToAddressServer();
    if (addrserver_connected)
    {
            if (mpRemoteCoreAddressHandler->numKnownServers() < 1)
            {
                mpRemoteCoreAddressHandler->requestAvailableServers();
            }

            // Now queue particles / models for remote evaluation
            mNumThreadsPerModel = numThreads;

            // Acquire slots and enqueue models
            int nServers = 0;
            int enqueCtr = 0;
            for (int m=0; m<models.size(); ++m)
            {
                QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(-1, mNumThreadsPerModel, mServerBlacklist);
                ModelWidget *pModel = models[m];

                // If we run out of servers, then enqueue models
                if (servers.isEmpty() && !mModelQueues.isEmpty())
                {
                    mModelQueues[enqueCtr].enqueue(pModel);

                    // Increment / reset counter
                    enqueCtr++;
                    if (enqueCtr >= mRemoteCoreSimulationHandlers.size())
                    {
                        enqueCtr = 0;
                    }
                }
                // Assign one model to each new simulation handler
                else if(!servers.isEmpty())
                {
                    QString server_addr = servers.front();
                    servers.pop_front();

                    SharedRemoteCoreSimulationHandlerT pSH(new RemoteCoreSimulationHandler());
                    pSH->setUserIdentification(gpConfig->getStringSetting(cfg::remotehopsanuseridentification));
                    pSH->setAddressServer(mpRemoteCoreAddressHandler->getAddressAndPort());
                    pSH->setHopsanServer(server_addr);
                    pSH->setNumThreads(mNumThreadsPerModel);
                    if (pSH->connect())
                    {
                        nServers++;
                        mRemoteCoreSimulationHandlers.append(pSH);

                        QQueue<ModelWidget*> q;
                        q.enqueue(pModel);
                        mModelQueues.append(q);
                    }
                    else
                    {
                        //if we fail to connect we should rerun this model the next step
                        m--;
                    }
                }
            }
            //! @todo need to make sure that all models were allocated properly
    }
}

bool RemoteSimulationQueueHandler::simulateModels(bool &rExternalReschedulingNeeded)
{
    rExternalReschedulingNeeded = false;
    return simulateModels();
}

bool RemoteSimulationQueueHandler::simulateModels()
{
    // Now start simulation of the first queued models
    QEventLoop event_loop;
    int h=0;
    QVector<MyBarrier*> barriers;
    barriers.resize(mRemoteCoreSimulationHandlers.size());
    for (int i=0; i<barriers.size(); ++i)
    {
        barriers[i] = new MyBarrier();
    }

    // Create a copy since we will dequeue and pop pointers
    QVector<QQueue<ModelWidget*>> modelQueues = mModelQueues;

    int prev_m=0;
    for (int m=0; m<mAllModels.size(); ++m)
    {
        // Block until next element in queue can be run
        if (barriers[h]->tryLock())
        {
            ModelWidget *pModel = modelQueues[h].dequeue();
            SharedRemoteCoreSimulationHandlerT pRCSH = mRemoteCoreSimulationHandlers[h];

            pModel->setExternalRemoteCoreSimulationHandler(pRCSH);
            // Load model remotely
            //! @todo handle failure
            bool rc = pModel->loadModelRemote();
            if (!rc)
            {
                return false;
            }
            // Disconnect old signals from unlock slot
            QObject::disconnect(0,0, barriers[h], SLOT(unlock()));
            // Connect this models signal to unlock slot
            QObject::connect(pModel, SIGNAL(simulationFinished()), barriers[h], SLOT(unlock()));

            // Start simulation
            rc = pModel->simulate_nonblocking();
            if (!rc)
            {
                return false;
            }

            // Increment counters
            h++;
            if (h >= mRemoteCoreSimulationHandlers.size())
            {
                h=0;
            }

            prev_m = m;
        }
        else
        {
            m = prev_m;
            event_loop.processEvents();
        }
    }

    // Block until all barriers free (all simulation done)
    for (int i=0; i<barriers.size(); ++i)
    {
        // If we can lock the mutex then we are done
        while (!barriers[i]->tryLock())
        {
            event_loop.processEvents();
        }
        barriers[i]->unlock();
        delete barriers[i];
    }

    return true;
}


void RemoteSimulationQueueHandler::determineBestSpeedup(int maxNumThreads, int maxPA, int &rPM, int &rPA, double &rSU)
{
    rPM = 1;
    rPA = 1;
    rSU = 1;

    bool addrserver_connected = connectToAddressServer();
    if (addrserver_connected)
    {
        mpRemoteCoreAddressHandler->requestAvailableServers();

        int nThreads = mNumThreadsPerModel;
        if (maxNumThreads < 0)
        {
            int nServers;
            mpRemoteCoreAddressHandler->getMaxNumSlots(nThreads, nServers);
        }

        // Limit num threads
        if (maxNumThreads > 0)
        {
            nThreads = qMin(maxNumThreads, nThreads);
        }

        QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(-1, nThreads, mServerBlacklist);


        // HOMOGENEOUS VERSION
        double bestSU=0;
        int bestPA=1, bestPM=1;
        const int nc = nThreads;
        const int np = servers.size(); // Num servers

        // Limit PA
        int pa_max = mAlgorithmMaxPa;
        if (maxPA > 0)
        {
            pa_max = qMin(pa_max, maxPA);
        }

        for (int pm=1; pm<=nc; ++pm)
        {
            //int pa_lim_max = qMin(pa_max, np *  (nc / pm) ); //Integer division is used to mimic floor (truncation)
            int pa_lim_max = pa_max;

            for (int pa_lim=1; pa_lim<=pa_lim_max; ++pa_lim)
            {

                //int pa = np * floor( nc / pm );
                //pa = qMin(pa, pa_lim); // Limit PA

                int pa = pa_lim;

                double sua = SUa ( pa, maxPA );
                double sum = SUm ( pm );
                double suq = SUq (pa , pm, np, nc );
                double SU =  sua * sum * suq ;

                gpMessageHandler->addInfoMessage(QString("Speedup: np: %1, pm: %2, pa: %3, SUm: %4, SUa: %5, SUq: %6, SU: %7").arg(np, 4).arg(pm, 4).arg(pa, 4).arg(sum, 10).arg(sua, 10).arg(suq, 10).arg(SU, 10));

                if ( SU>bestSU )
                {
                    bestSU = SU;
                    bestPM = pm;
                    bestPA = pa;
                }
            }
        }

        gpMessageHandler->addInfoMessage(QString("Best Speedup: pm: %1, pa: %2, SU: %3").arg(bestPM, 4).arg(bestPA, 4).arg(bestSU, 10));

        rPM = bestPM;
        rPA = bestPA;
        rSU = bestSU;
    }
}

bool RemoteSimulationQueueHandler::hasServers() const
{
    return !mRemoteCoreSimulationHandlers.isEmpty();
}

bool RemoteSimulationQueueHandler::hasQueues() const
{
    return !mModelQueues.isEmpty();
}

bool RemoteSimulationQueueHandler::connectToAddressServer()
{
    // Get address server handler
    if(mpRemoteCoreAddressHandler.isNull())
    {
        mpRemoteCoreAddressHandler = getSharedRemoteCoreAddressHandler();
    }

    // Disconnect if address has changed
    if (mpRemoteCoreAddressHandler->getAddressAndPort() != gpConfig->getStringSetting(cfg::remotehopsanaddresserveraddress))
    {
        mpRemoteCoreAddressHandler->disconnect();
    }

    // Connect unless already connected
    bool addrserver_connected = mpRemoteCoreAddressHandler->isConnected();
    if (!addrserver_connected)
    {
        mpRemoteCoreAddressHandler->setHopsanAddressServer(gpConfig->getStringSetting(cfg::remotehopsanaddresserveraddress));
        addrserver_connected = mpRemoteCoreAddressHandler->connect();
    }

    return addrserver_connected;
}

double RemoteSimulationQueueHandler::SUa(int numParallellEvaluators, int numParticles)
{
    Q_UNUSED(numParallellEvaluators)
    Q_UNUSED(numParticles)
    return 1;
}

double RemoteSimulationQueueHandler::SUq(int Pa, int Pm, int np, int nc)
{
    int nAvailableQueues = np * (1+(nc-Pm));
    if (nAvailableQueues < 1)
    {
        return 0;
    }
    else
    {
        //int intDiv = Pa / nAvailableQueues;
        //int intDivRem = Pa % nAvailableQueues;

        //int maxNumJobsInQueue = Pa / nAvailableQueues + (Pa % nAvailableQueues);
        //int maxNumJobsInQueue = intDiv + 1*(intDivRem != 0);
        double maxNumJobsInQueueD = ceil(double(Pa) / double(nAvailableQueues));

        return 1.0/double(maxNumJobsInQueueD);
    }
}

void RemoteSimulationQueueHandler::reset()
{
    // Reset (to NULL) the shared pointers first, before clearing
    for (ModelWidget *pModel : mAllModels)
    {
        pModel->setExternalRemoteCoreSimulationHandler(SharedRemoteCoreSimulationHandlerT());
    }
    mAllModels.clear();
    mModelQueues.clear();
    mRemoteCoreSimulationHandlers.clear();
    mpRemoteCoreAddressHandler.clear();
}

void RemoteSimulationQueueHandler::clear()
{
    reset();
    mServerBlacklist.clear();
}

void RemoteSimulationQueueHandler::benchmarkModel(ModelWidget *pModel)
{
    bool addrserver_connected = connectToAddressServer();
    if (addrserver_connected)
    {
        // OK first get the fastest machine and benchmark nThreads
        int maxNumSlots, nServersWithMaxSlots;
        mpRemoteCoreAddressHandler->requestAvailableServers();
        mpRemoteCoreAddressHandler->getMaxNumSlots(maxNumSlots, nServersWithMaxSlots);
        if (nServersWithMaxSlots > 0)
        {
            mNumThreadsVsModelEvalTime.clear();
            mNumThreadsVsModelEvalTime.reserve(maxNumSlots);

            // Benchmark evalTime vs numThreads
            QString server_addr = mpRemoteCoreAddressHandler->getBestAvailableServer(maxNumSlots, mServerBlacklist);
            SharedRemoteCoreSimulationHandlerT pRCSH(new RemoteCoreSimulationHandler());
            pRCSH->setUserIdentification(gpConfig->getStringSetting(cfg::remotehopsanuseridentification));
            pRCSH->setAddressServer(mpRemoteCoreAddressHandler->getAddressAndPort());
            pRCSH->setHopsanServer(server_addr);
            bool rc = pRCSH->connectServer();
            if (rc)
            {
                QDomDocument doc = pModel->saveToDom();
                for (int t=1; t<=maxNumSlots; ++t)
                {
                    double simTime;
                    bool rc2 = pRCSH->benchmarkModel_blocking(doc.toString(), t, simTime);
                    if(rc2)
                    {
                        mNumThreadsVsModelEvalTime.push_back(simTime);
                        gpMessageHandler->addInfoMessage(QString("Benchmark: nThreads %1, evalTime: %2").arg(t).arg(simTime, 10));
                    }
                    else
                    {
                        gpMessageHandler->addErrorMessage(QString("Failed remote benchmark, using numThreads: %1").arg(t));
                    }
                }
            }
            pRCSH->disconnect();
        }
    }
}

bool RemoteSimulationQueueHandlerLB::simulateModels(bool &rExternalReschedulingNeeded)
{
    rExternalReschedulingNeeded = false;

    QTime timer;
    timer.start();

    bool someServerSlowdownProblem = true;
    bool remoteFailure = false;
    QEventLoop event_loop;

    // Loop until there is no problem, hopefully this will only be one loop
    while (someServerSlowdownProblem)
    {
        const int numQueues = mRemoteCoreSimulationHandlers.size();
        someServerSlowdownProblem = false;

        // Create a copy since we will dequeue and pop pointers
        QVector<QQueue<ModelWidget*>> modelQueues = mModelQueues;

        // This semaphore will asynchronously keep track on each queues availability
        MySemaphore numQueuesSemaphore(numQueues);
        // A vector of the models currently being simulated
        QVector<ModelWidget*> modelsInProgress;
        // A vector of the simulation progress for the  models currently being simulated
        QVector<MyProgressTracker> modelsInProgressProgress;

        int numProcessedModels=0;
        while (numProcessedModels < mAllModels.size() || (numQueuesSemaphore.available() != numQueues))
        {
            // Abort if no simulation resources exist
            if (numQueues == 0)
            {
                break;
            }

            // If all queues are available then start the remote parallel simulation of "numQueues" models
            if (numQueuesSemaphore.available() == numQueues)
            {
                // Disconnect any existing "simulationFinished" signals from the numQueuesSemaphore unlock slot
                for (int m=0; m<modelsInProgress.size(); ++m)
                {
                    QObject::disconnect(modelsInProgress[m], SIGNAL(simulationFinished()), &numQueuesSemaphore, SLOT(releaseSlot()));
                }

                // Reinitialize "in progress" vectors
                modelsInProgress.clear();
                modelsInProgress.reserve(numQueues);
                modelsInProgressProgress.clear();
                modelsInProgressProgress.resize(numQueues);
                for (MyProgressTracker &pt : modelsInProgressProgress)
                {
                    //! @todo not hard coded 10
                    pt.initialize(10);
                }


                // Prepare simulation of the "first in line" models in each queue
                for (int q=0; q<modelQueues.size(); ++q)
                {
                    if (!modelQueues[q].empty())
                    {
                        ModelWidget *pModel = modelQueues[q].dequeue();
                        modelsInProgress.push_back(pModel);
                        SharedRemoteCoreSimulationHandlerT pRCSH = mRemoteCoreSimulationHandlers[q];

                        // Assign remote simulator wrapper class to models
                        pModel->setExternalRemoteCoreSimulationHandler(pRCSH);
                        // Load model remotely
                        bool rc = pModel->loadModelRemote();
                        if (rc)
                        {
                            // Connect this models "finished" signal to semaphore unlock slot
                            QObject::connect(pModel, SIGNAL(simulationFinished()), &numQueuesSemaphore, SLOT(releaseSlot()));
                        }
                        else
                        {
                            //! @todo handle failure
                            remoteFailure = true;
                            return false;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                // Start nonblocking remote simulation of each queued model
                for (int m=0; m<modelsInProgress.size(); ++m)
                {
                    //! @todo they are not started in parallel exactly which may cause problems if we have manny particles
                    bool rc = modelsInProgress[m]->simulate_nonblocking();
                    // If the simulation was initiated successfully then acquire one semaphore lock to indicate that one resource is taken
                    if (rc)
                    {
                        numQueuesSemaphore.acquire();
                        numProcessedModels++;
                    }
                    // If simulation would not start then abort
                    else
                    {
                        remoteFailure = true;
                        return false;
                    }
                }
            }
            // If all queues are not available for parallel simulation, then monitor the progress in order to detect slowdowns or lost resources
            else
            {
                event_loop.processEvents();

                // If all are running then monitor progress
                //! @todo right now ,slightly unfair since models are not started at exactly the same time

                const int numModelsInProgress = modelsInProgress.size();

                for (int m=0; m<numModelsInProgress; ++m)
                {
                    //! @todo not hard coded 1 second
                    if (modelsInProgressProgress[m].secondsSinceLastAdd() > 0.5)
                    {
                        ModelWidget *pModel = modelsInProgress[m];
                        //! @todo this is unfair as getSimulationProgress is blocking (should maybe get estimated time remaining, instead)
                        double progress = pModel->getSimulationProgress();
                        modelsInProgressProgress[m].addProgress(progress);
                        modelsInProgressProgress[m].estimateRemainingTime();
                    }
                }

                double bestEST=1e200;
                double bestProgress=0;
                QString eststr="est ", progressstr="prog ";
                for (int m=0; m<numModelsInProgress; ++m)
                {
                    double progress = modelsInProgressProgress[m].lastTrackedProgress();
                    // Get ETA, Note! eta<0 if it could not be calculated
                    double est = modelsInProgressProgress[m].lastEstimatedTotalSimtime();

                    eststr.append(QString("%1 ").arg(est));
                    progressstr.append(QString("%1 ").arg(progress));
                    if ( (est>=0) && (est<bestEST))
                    {
                        bestEST = est;
                    }
                    if (progress > bestProgress)
                    {
                        bestProgress = progress;
                    }
                }
                gpMessageHandler->addInfoMessage(eststr);
                gpMessageHandler->addInfoMessage(progressstr);

                // Calculate progress difference
                double maxESTDiff=0;
                QVector<double> estDiffs;
                QVector<double> progressDiffs;
                estDiffs.reserve(numModelsInProgress);
                progressDiffs.reserve(numModelsInProgress);
                for (int i=0; i<numModelsInProgress; ++i)
                {
                    double esti = modelsInProgressProgress[i].lastEstimatedTotalSimtime();
                    if (esti>=0)
                    {
                        double estDiffi = (esti-bestEST);
                        double progressDiff = (modelsInProgressProgress[i].lastTrackedProgress()-bestProgress);
                        estDiffs.push_back( estDiffi );
                        progressDiffs.push_back( progressDiff );
                        if ( fabs(estDiffi) > maxESTDiff)
                        {
                            maxESTDiff = fabs(estDiffi);
                        }
                    }
                    //qDebug() << "Parallel diffs: " << diffs;
                }

                // Get expected evaluation time
//                double expecteadSimulationTime=-1;
//                //! @todo this is a bit scary since the benchmark time may not be accurate (also we may not benchmark the entire model, so the benchmark results need to know about this)
//                if (mNumThreadsVsModelEvalTime.size()>=mNumThreadsPerModel && mNumThreadsPerModel>0)
//                {
//                    expecteadSimulationTime = mNumThreadsVsModelEvalTime[mNumThreadsPerModel-1];
//                }

                if (bestEST < 1e199)
                {

#define ESTDEFAULTDIFFTHRESH 60.0
#define ESTMINDIFFTHRESH 5.0
#define ESTDIFFTHRESHSCALE 2.0
                    //! @todo this wont work, need to estimate total simulation time for this to be relevant
                    double maxEstDiffThreshold = ESTDEFAULTDIFFTHRESH;
//                    if (expecteadSimulationTime > 0)
//                    {
                    maxEstDiffThreshold = qMax(bestEST*ESTDIFFTHRESHSCALE, ESTMINDIFFTHRESH);
//                    }

                    if (maxESTDiff > maxEstDiffThreshold)
                    {
                        for (int i=0; i<estDiffs.size(); ++i)
                        {
                            // Blacklist servers trailing behind
                            if ( estDiffs[i] > maxEstDiffThreshold)
                            {
                                someServerSlowdownProblem = true;
                                mServerBlacklist.append(mRemoteCoreSimulationHandlers[i]->getHopsanServerAddress());
                            }
                        }
                    }
                }

//                double bestProgress=0;
//                for (int m=0; m<modelsInProgress.size(); ++m)
//                {
//                    ModelWidget *pModel = modelsInProgress[m];
//                    //! @todo this is unfair as getSimulationProgress is blocking (should maybe get estimated time remaining, instead)
//                    double progress = pModel->getSimulationProgress();
//                    modelsInProgressProgress[m] = progress;
//                    if (progress > bestProgress)
//                    {
//                        bestProgress = progress;
//                    }
//                }

//                // Calc progress difference
//                double maxDiff=0;
//                QVector<double> diffs;
//                diffs.reserve(modelsInProgress.size());
//                for (int i=0; i<modelsInProgress.size(); ++i)
//                {
//                    double diff = (modelsInProgressProgress[i]-bestProgress)*100;
//                    diffs.push_back( diff );
//                    if ( fabs(diff) > maxDiff)
//                    {
//                        maxDiff = fabs(diff);
//                    }
//                    //qDebug() << "Parallel diffs: " << diffs;
//                }

//#define MAXDIFFTHRESH 50
//                //! @todo not hard coded threshold, use smarter method
//                if (maxDiff > MAXDIFFTHRESH)
//                {
//                    someServerSlowdownProblem = true;
//                    for (int i=0; i<diffs.size(); ++i)
//                    {
//                        // Blacklist servers trailing behind
//                        if (diffs[i] < -MAXDIFFTHRESH)
//                        {
//                            mServerBlacklist.append(mRemoteCoreSimulationHandlers[i]->getHopsanServerAddress());
//                        }
//                    }
//                }

                // Terminate simulation
                if (someServerSlowdownProblem)
                {
                    for (int m=0; m<modelsInProgress.size(); ++m)
                    {
                        mRemoteCoreSimulationHandlers[m]->abortSimulation();
                        //ModelWidget *pModel = modelsInProgress[m];
                        //pModel->abortSimulation();
                    }

                    break;
                }
            }
        }

        // Wait until all simulations finished, and data collected and such things
        while (numQueuesSemaphore.available() != numQueues)
        {
            event_loop.processEvents();
        }

        // Now all communication with worker should be finished, lets disconnect
        if (someServerSlowdownProblem)
        {
            for (int m=0; m<mRemoteCoreSimulationHandlers.size(); ++m)
            {
                mRemoteCoreSimulationHandlers[m]->disconnect();
            }
        }

        // IF we faced a problem then try to reschedule all work
        //! @todo rescheduling everything is probably not the best solution
        if (someServerSlowdownProblem)
        {
            if (mReschedulingMethod == ExternalRescheduling)
            {
                int pm,pa;
                double su;
                determineBestSpeedup(-1, -1, pm, pa, su);
                if (pa != mAllModels.size())
                {
                    rExternalReschedulingNeeded = true;
                    gpMessageHandler->addInfoMessage("Note! Rescheduling of optimization algorithm needed");
                }

                // Do internal load balancing
                // Note! We do not need to run benchmark again
                gpMessageHandler->addInfoMessage("Using internal load balancing");
                setupModelQueues(mAllModels, pm);

//                int pm,pa;
//                double su;
//                determineBestSpeedup(-1, -1, pm, pa, su);
//                if (pa != mAllModels.size())
//                {
//                    rExternalReschedulingNeeded = true;
//                    gpMessageHandler->addInfoMessage("Aborting iteration, rescheduling by optimization algorithm needed");
//                    return false;
//                }
//                else
//                {
//                    // Do internal load balancing only
//                    // Note! We do not need to run benchmark again
//                    gpMessageHandler->addInfoMessage("External rescheduling is not needed, using internal load balancing");
//                    setupModelQueues(mAllModels, pm);
//                }
            }
            else if (mReschedulingMethod == InternalLoadBalance)
            {
                int pm,pa;
                double su;
                determineBestSpeedup(-1, mAllModels.size(), pm, pa, su);
                // Do internal load balancing only
                // Note! We do not need to run benchmark again
                setupModelQueues(mAllModels, pm);
            }

        }

        // We have run out of resources, exit with failure
        if (numQueues == 0)
        {
            return false;
        }

        //! @todo need to abort this loop if all servers stop responding, now we are stuck forever
    }
    int elapsed = timer.elapsed();
    gpMessageHandler->addInfoMessage(QString("One full iteration took: %1 ms").arg(elapsed));
    return true;
}

bool RemoteSimulationQueueHandlerCRFP::simulateModels()
{
    // In CRFP This function should not be called
    Q_ASSERT(false);
    return false;
}


bool RemoteSimulationQueueHandlerLB::simulateModels()
{
    bool dummy;
    return simulateModels(dummy);
}


double RemoteSimulationQueueHandler_Homo_Re_PSO::SUa(int numParallellEvaluators, int numParticles)
{
    if (numParallellEvaluators > 0)
    {
        // If integer division has remainder, then balancing is uneven, we need one additional evaluation run (not all evaluators will be used)
        int maxQueueLength = numParticles / numParallellEvaluators;
        if ( (numParticles % numParallellEvaluators) != 0)
        {
            maxQueueLength += 1.0;
        }
        return double(numParticles) / double(maxQueueLength);
    }
    return 0;
}

double RemoteSimulationQueueHandler::SUm(int nThreads)
{
    // Convert to zero-based index
    nThreads--;

    if ( !mNumThreadsVsModelEvalTime.isEmpty() && (nThreads >= 0) )
    {
        double oneCoreEvalTime = mNumThreadsVsModelEvalTime.first();
        if (nThreads < mNumThreadsVsModelEvalTime.size())
        {
            return oneCoreEvalTime / mNumThreadsVsModelEvalTime[nThreads];
        }
        //! @todo what if nThreads to high, right now we pretend no speedup
    }
    return 0;
}


double RemoteSimulationQueueHandler_Homo_Re_CRFP0::SUa(int numParallellEvaluators, int numParticles)
{
    Q_UNUSED(numParticles)

    // Convert to zero-based index
    numParallellEvaluators--;

    // Need to lookup from table, approximate speedup for algorithms, based on method
    QVector<double> sua {1.0, 1.43, 1.78, 1.81, 1.87, 1.99, 1.91, 1.92};
    if (numParallellEvaluators >= 0)
    {
        if  (numParallellEvaluators < sua.size())
        {
            return sua[numParallellEvaluators];
        }
        else
        {
            return sua.back(); // Assume same speedup as last known
        }
    }
    return 0.;
}


double RemoteSimulationQueueHandler_Homo_Re_CRFP1::SUa(int numParallellEvaluators, int numParticles)
{
    Q_UNUSED(numParticles)

    // Convert to zero-based index
    numParallellEvaluators--;

    // Need to lookup from table, approximate speedup for algorithms, based on method
    QVector<double> sua {1.0, 1.35, 1.52, 1.58, 1.72, 1.74, 1.78, 1.79};
    if (numParallellEvaluators >= 0)
    {
        if  (numParallellEvaluators < sua.size())
        {
            return sua[numParallellEvaluators];
        }
        else
        {
            return sua.back(); // Assume same speedup as last known
        }
    }
    return 0.;
}

double RemoteSimulationQueueHandler_Homo_Re_CRFP1::SUq(int Pa, int Pm, int np, int nc)
{
    Q_UNUSED(Pa) Q_UNUSED(Pm) Q_UNUSED(np) Q_UNUSED(nc)
    return 1.0;
}



RemoteSimulationQueueHandler* createRemoteSimulationQueueHandler(RemoteSimulationQueueHandlerType type)
{
    if (type == Basic)
    {
        return new RemoteSimulationQueueHandler();
    }
    else if (type == SensitivityAnalysis)
    {
        return new RemoteSimulationQueueHandlerSA();
    }
    else if (type == Pso_Homo_Reschedule)
    {
        return new RemoteSimulationQueueHandler_Homo_Re_PSO();
    }
    else if (type == Crfp0_Homo_Reschedule)
    {
        return new RemoteSimulationQueueHandler_Homo_Re_CRFP0();
    }
    else if (type == Crfp1_Homo_Reschedule)
    {
        return new RemoteSimulationQueueHandler_Homo_Re_CRFP1();
    }
    else
    {
        return nullptr;
    }
}

void removeRemoteSimulationQueueHandler(RemoteSimulationQueueHandler *pHandler)
{
    if (pHandler)
    {
        delete pHandler;
    }
}


#include "RemoteSimulationUtils.moc"

#endif



