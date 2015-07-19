/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

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

RemoteModelSimulationQueuer::~RemoteModelSimulationQueuer()
{
    clear();
}

void RemoteModelSimulationQueuer::setupModelQueues(QVector<ModelWidget *> models, int numThreads)
{
    reset();
    mAllModels = models;

    bool addrserver_connected = connectToAddressServer();
    if (addrserver_connected)
    {
            // Now queue particles / models for remote evaluation
            mNumThreadsPerModel = numThreads;

            // Aquire slots and enque models
            int nServers = 0;
            int enqueCtr = 0;
            for (int m=0; m<models.size(); ++m)
            {
                QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(-1, mNumThreadsPerModel, mServerBlacklist);
                ModelWidget *pModel = models[m];

                // If we run out of servers, then enque models
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

bool RemoteModelSimulationQueuer::simulateModels(bool &rExternalReschedulingNeeded)
{
    rExternalReschedulingNeeded = false;
    return simulateModels();
}

bool RemoteModelSimulationQueuer::simulateModels()
{
    // Now start simulation of the first qued models
    QEventLoop event_loop;
    int h=0;
    QVector<MyBarrier*> barriers;
    barriers.resize(mRemoteCoreSimulationHandlers.size());
    for (int i=0; i<barriers.size(); ++i)
    {
        barriers[i] = new MyBarrier();
    }

    // Create a copy since we will deque and pop pointers
    QVector<QQueue<ModelWidget*>> modelQueues = mModelQueues;

    int prev_m=0;
    for (int m=0; m<mAllModels.size(); ++m)
    {
        // Block until next element in queue can be run
        if (barriers[h]->tryLock())
        {
            ModelWidget *pModel = modelQueues[h].dequeue();
            SharedRemoteCoreSimulationHandlerT pRCSH = mRemoteCoreSimulationHandlers[h];

            pModel->setUseRemoteSimulationCore(pRCSH);
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

    // Block untill all bariers free (all simulation done)
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


void RemoteModelSimulationQueuer::determineBestSpeedup(int maxNumThreads, int maxPA, int &rPM, int &rPA, double &rSU)
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

bool RemoteModelSimulationQueuer::hasServers() const
{
    return !mRemoteCoreSimulationHandlers.isEmpty();
}

bool RemoteModelSimulationQueuer::connectToAddressServer()
{
    // Get address server handler
    if(mpRemoteCoreAddressHandler.isNull())
    {
        mpRemoteCoreAddressHandler = getSharedRemoteCoreAddressHandler();
    }

    // Disconnect if address has changed
    if (mpRemoteCoreAddressHandler->getAddressAndPort() != gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESSSERVERADDRESS))
    {
        mpRemoteCoreAddressHandler->disconnect();
    }

    // Connect unless already connected
    bool addrserver_connected = mpRemoteCoreAddressHandler->isConnected();
    if (!addrserver_connected)
    {
        mpRemoteCoreAddressHandler->setHopsanAddressServer(gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESSSERVERADDRESS));
        addrserver_connected = mpRemoteCoreAddressHandler->connect();
    }

    return addrserver_connected;
}

double RemoteModelSimulationQueuer::SUa(int numParallellEvaluators, int numParticles)
{
    return 1;
}

double RemoteModelSimulationQueuer::SUq(int Pa, int Pm, int np, int nc)
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

void RemoteModelSimulationQueuer::reset()
{
    // Reset (to NULL) the shared pointers first, before clearing
    for (ModelWidget *pModel : mAllModels)
    {
        pModel->setUseRemoteSimulationCore(SharedRemoteCoreSimulationHandlerT());
    }
    mAllModels.clear();
    mModelQueues.clear();
    mRemoteCoreSimulationHandlers.clear();
    mpRemoteCoreAddressHandler.clear();
}

void RemoteModelSimulationQueuer::clear()
{
    reset();
    mServerBlacklist.clear();
}

void RemoteModelSimulationQueuer::benchmarkModel(ModelWidget *pModel)
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

bool RemoteModelSimulationQueuerLB::simulateModels(bool &rExternalReschedulingNeeded)
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

        // Create a copy since we will deque and pop pointers
        QVector<QQueue<ModelWidget*>> modelQueues = mModelQueues;

        MySemaphore semaphore(numQueues);
        QVector<ModelWidget*> modelsInProgress;
        QVector<double> modelsInProgressProgress;

        int numProcessedModels=0;
        while (numProcessedModels < mAllModels.size() || (semaphore.available() != numQueues))
        {
            // Abort if no simulation resources exist
            if (numQueues == 0)
            {
                break;
            }

            if (semaphore.available() == numQueues)
            {
                for (int m=0; m<modelsInProgress.size(); ++m)
                {
                    // Disconnect old signals from unlock slot
                    QObject::disconnect(modelsInProgress[m], SIGNAL(simulationFinished()), &semaphore, SLOT(releaseSlot()));
                }

                modelsInProgress.clear();
                modelsInProgress.reserve(numQueues);
                modelsInProgressProgress.clear();
                modelsInProgressProgress.reserve(numQueues);

                // Prepare simulation of the first in line in the queues
                // Set handlers to models and hanlde signal slot connections
                for (int q=0; q<modelQueues.size(); ++q)
                {
                    if (!modelQueues[q].empty())
                    {
                        ModelWidget *pModel = modelQueues[q].dequeue();

                        modelsInProgress.push_back(pModel);
                        modelsInProgressProgress.push_back(0);
                        SharedRemoteCoreSimulationHandlerT pRCSH = mRemoteCoreSimulationHandlers[q];

                        pModel->setUseRemoteSimulationCore(pRCSH);
                        // Load model remotely
                        bool rc = pModel->loadModelRemote();
                        if (rc)
                        {
                            // Connect this models signal to unlock slot
                            QObject::connect(pModel, SIGNAL(simulationFinished()), &semaphore, SLOT(releaseSlot()));
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

                // Start simulation
                for (int m=0; m<modelsInProgress.size(); ++m)
                {
                    //! @todo they are not started in paralell exactly which may cause problems if we have manny particles
                    bool rc = modelsInProgress[m]->simulate_nonblocking();
                    if (rc)
                    {
                        semaphore.acquire();
                        numProcessedModels++;
                    }
                    else
                    {
                        remoteFailure = true;
                        return false;
                    }
                }
            }
            else
            {
                event_loop.processEvents();

                // If all are running then monitor progress
                //! @todo right now ,slighlty unfair since models are not started at exactly the same time

                double bestProgress=0;
                for (int m=0; m<modelsInProgress.size(); ++m)
                {
                    ModelWidget *pModel = modelsInProgress[m];
                    //! @todo this is unfair as getSimualtionProgress is blocking (should maybe get estimated time remaining, instead)
                    double progress = pModel->getSimulationProgress();
                    modelsInProgressProgress[m] = progress;
                    if (progress > bestProgress)
                    {
                        bestProgress = progress;
                    }
                }

                // Calc progress difference
                double maxDiff=0;
                QVector<double> diffs;
                diffs.reserve(modelsInProgress.size());
                for (int i=0; i<modelsInProgress.size(); ++i)
                {
                    double diff = (modelsInProgressProgress[i]-bestProgress)*100;
                    diffs.push_back( diff );
                    if ( fabs(diff) > maxDiff)
                    {
                        maxDiff = fabs(diff);
                    }
                    //qDebug() << "Parallel diffs: " << diffs;
                }

#define MAXDIFFTHRESH 50
                //! @todo not hardcoded threshold, use smarter method
                if (maxDiff > MAXDIFFTHRESH)
                {
                    someServerSlowdownProblem = true;
                    for (int i=0; i<diffs.size(); ++i)
                    {
                        // Blacklist servers trailing behind
                        if (diffs[i] < -MAXDIFFTHRESH)
                        {
                            mServerBlacklist.append(mRemoteCoreSimulationHandlers[i]->getHopsanServerAddress());
                        }
                    }
                }

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

        // Wait until all simualtions finnished, and data collected and such things
        while (semaphore.available() != numQueues)
        {
            event_loop.processEvents();
        }

        // Now all communication with worker should be finnished, lets disconnect
        if (someServerSlowdownProblem)
        {
            for (int m=0; m<modelsInProgress.size(); ++m)
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
                    gpMessageHandler->addInfoMessage("Aborting iteration, rescheduling by optimization algorithm needed");
                    return false;
                }
                else
                {
                    // Do internal load balancing only
                    // Note! We do not need to run benchmark again
                    gpMessageHandler->addInfoMessage("External rescheduling is not needed, using internal load balancing");
                    setupModelQueues(mAllModels, pm);
                }
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

bool RemoteModelSimulationQueuerCRFP::simulateModels()
{
    // In CRFP This function should not be called
    Q_ASSERT(false);
    return false;
}


bool RemoteModelSimulationQueuerLB::simulateModels()
{
    bool dummy;
    return simulateModels(dummy);
}


double RemoteModelSimulationQueuer_Homo_Re_PSO::SUa(int numParallellEvaluators, int numParticles)
{
    if (numParallellEvaluators > 0)
    {
        // If integer division has remanider, then ballancing is uneven, we need one addtional evaluation run (not all evaluatros will be used)
        int maxQueueLength = numParticles / numParallellEvaluators;
        if ( (numParticles % numParallellEvaluators) != 0)
        {
            maxQueueLength += 1.0;
        }
        return double(numParticles) / double(maxQueueLength);
    }
    return 0;
}

double RemoteModelSimulationQueuer::SUm(int nThreads)
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


double RemoteModelSimulationQueuer_Homo_Re_CRFP0::SUa(int numParallellEvaluators, int numParticles)
{
    Q_UNUSED(numParticles)

    // Convert to zero-based index
    numParallellEvaluators--;

    // Need to lookup from table, approximate speedup for algorithms, absed on method
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


double RemoteModelSimulationQueuer_Homo_Re_CRFP1::SUa(int numParallellEvaluators, int numParticles)
{
    Q_UNUSED(numParticles)

    // Convert to zero-based index
    numParallellEvaluators--;

    // Need to lookup from table, approximate speedup for algorithms, absed on method
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

double RemoteModelSimulationQueuer_Homo_Re_CRFP1::SUq(int Pa, int Pm, int np, int nc)
{
    Q_UNUSED(Pa) Q_UNUSED(Pm) Q_UNUSED(np) Q_UNUSED(nc)
    return 1.0;
}



void chooseRemoteModelSimulationQueuer(RemoteModelSimulationQueuerType type)
{
    if (gpRemoteModelSimulationQueuer)
    {
        delete gpRemoteModelSimulationQueuer;
    }

    if (type == Basic)
    {
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer();
    }
    else if (type == Pso_Homo_Reschedule)
    {
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer_Homo_Re_PSO();
    }
    else if (type == Crfp0_Homo_Reschedule)
    {
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer_Homo_Re_CRFP0();
    }
    else if (type == Crfp1_Homo_Reschedule)
    {
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer_Homo_Re_CRFP1();
    }
}

RemoteModelSimulationQueuer *gpRemoteModelSimulationQueuer=0;

#include "RemoteSimulationUtils.moc"

#endif
