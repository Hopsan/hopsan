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


//double SUa_cpxrfp(int numParallellEvaluators, int method)
//{
//    // Need to lookup from table, approximate speedup for algorithms, absed on method
//    QVector<double> sua;
//    if (method == 1)
//    {
//        sua = {1.0, 1.43, 1.78, 1.81, 1.87, 1.99, 1.91, 1.92};
//    }
//    else if (method == 2)
//    {
//        sua = {1.0, 1.35, 1.52, 1.58, 1.72, 1.74, 1.78, 1.79};
//    }

//    if (numParallellEvaluators <= sua.size())
//    {
//        return sua[numParallellEvaluators];
//    }

//    return 0.;
//}

//double SUa_pso(int numParallellEvaluators, int numParticles)
//{
//    if (numParallellEvaluators > 0)
//    {
//        int maxParticlesPerEvaluator = numParticles / numParallellEvaluators;

//        // If integer division has remanider, then ballancing is uneven, we need one addtional evaluation run (not all evaluatros will be used)
//        if (numParticles % numParallellEvaluators != 0)
//        {
//            maxParticlesPerEvaluator++;
//        }
//        return double(numParticles) / double(maxParticlesPerEvaluator);
//    }
//    return 0;
//}


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

void RemoteModelSimulationQueuer::setup(QVector<ModelWidget *> models)
{
    reset();
    mAllModels = models;

    bool addrserver_connected = connectToAddressServer();
    if (addrserver_connected)
    {
        mpRemoteCoreAddressHandler->requestAvailableServers();
        QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(1e200, mNumThreadsPerModel);

        if (!servers.isEmpty())
        {
            accuireSlotsAndEnqueModels(models, mNumThreadsPerModel);
        }
    }
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
            // Disconnect old signals from unlock slot
            QObject::disconnect(0,0, barriers[h], SLOT(unlock()));
            // Connect this models signal to unlock slot
            QObject::connect(pModel, SIGNAL(simulationFinished()), barriers[h], SLOT(unlock()));

            // Start simulation
            pModel->simulate_nonblocking();

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

void RemoteModelSimulationQueuer::accuireSlotsAndEnqueModels(QVector<ModelWidget *> models, int numThreads)
{
    int nServers = 0;
    int enqueCtr = 0;
    for (int m=0; m<models.size(); ++m)
    {
        QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(1e200, numThreads, mServerBlacklist);
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

void RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE::setup(QVector<ModelWidget *> models)
{
    reset();
    mAllModels = models;

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

            // Benchamrk speed vs num threads
            QString server_addr = mpRemoteCoreAddressHandler->getBestAvailableServer(maxNumSlots, mServerBlacklist);
            SharedRemoteCoreSimulationHandlerT pRCSH(new RemoteCoreSimulationHandler());
            pRCSH->setHopsanServer(server_addr);
            bool rc = pRCSH->connectServer();
            if (rc)
            {
                ModelWidget *pModel = models.first();
                QDomDocument doc = pModel->saveToDom();

                for (int t=1; t<=maxNumSlots; ++t)
                {
                    double simTime;
                    bool rc2 = pRCSH->benchmarkModel_blocking(doc.toString(), t, simTime);
                    if(rc2)
                    {
                        mNumThreadsVsModelEvalTime.push_back(simTime);
                        gpMessageHandler->addInfoMessage(QString("Benchmark: nThreads %1, speed: %2").arg(t).arg(simTime));
                    }
                    else
                    {
                        qDebug() << "Failed remote benchamrk, using numThreads: " << t;
                    }
                }
            }
            pRCSH->disconnect();
            qDebug() << "NumThreads speedup: " << mNumThreadsVsModelEvalTime;

            // Now determine optimal allocation
            QList<QString> matching_servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(1e200, maxNumSlots, mServerBlacklist);
            double SU;
            int pa, pm;
            determineBestSpeedup(models.size(), maxNumSlots, matching_servers.size(), pm, pa, SU);
            qDebug() << "Best speedup: " << pm << " " << pa << " " << SU;

            // Now queue particles / models for remote evaluation
            mNumThreadsPerModel = pm;
            accuireSlotsAndEnqueModels(models, mNumThreadsPerModel);




        }
    }
}

bool RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE::simulateModels()
{
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
                //! @todo not hardcoded threshold
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
                        ModelWidget *pModel = modelsInProgress[m];
                        //! @todo abort simulation
                        //pModel->abortSimulation();
                        //mRemoteCoreSimulationHandlers[m]->disconnect();
                    }

                    break;
                }
            }
        }

        // Wait until all simualtions finnished, (should not need to wait here, but lets wait just in case)
        while (semaphore.available() != numQueues)
        {
            event_loop.processEvents();
        }

        //! @todo move disconnect here due to problems
        if (someServerSlowdownProblem)
        {
            for (int m=0; m<modelsInProgress.size(); ++m)
            {
                ModelWidget *pModel = modelsInProgress[m];
                //! @todo abort simulation
                //pModel->abortSimulation();
                mRemoteCoreSimulationHandlers[m]->disconnect();
            }
        }

        // IF we faced a problem then try to reschedule all work
        //! @todo rescheduling everything is probably not the best solution
        if (someServerSlowdownProblem)
        {
            setup(mAllModels);
        }

        // We have run out of resources, exit with failure
        if (numQueues == 0)
        {
            return false;
        }

        //! @todo need to abort this loop if all servers stop responding, now we are stuck forever
    }
    int elapsed = timer.elapsed();
    gpMessageHandler->addInfoMessage(QString("One full itteration took: %1 ms").arg(elapsed));
    return true;
}


void RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE::determineBestSpeedup(int numParticles, int maxNumThreads, int numServers, int &rPM, int &rPA, double &rSU)
{
    // HOMOGENEOUS VERSION

    double bestSU=0;
    int bestPA=1, bestPM=1;
    const int nc = maxNumThreads;
    const int np = numServers;

    for (int pm=1; pm<=nc; ++pm)
    {
        int pa = np * floor( nc / pm );
        double sua = SUa( pa, numParticles  );
        double sum = SUm ( pm );
        double SU =  sua * sum ;

        gpMessageHandler->addInfoMessage(QString("Speedup: pm: %1, pa: %2, SUm: %3, SUa: %4, SU: %5").arg(pm).arg(pa).arg(sum).arg(sua).arg(SU));
        qDebug() << "Speedup: " << pm << " " << pa << " " << sum << " " << sua << " " << SU;

        if ( SU>bestSU )
        {
            bestSU = SU;
            bestPM = pm;
            bestPA = pa;
        }
    }

    gpMessageHandler->addInfoMessage(QString("Best Speedup: pm: %1, pa: %2, SU: %3").arg(bestPM).arg(bestPA).arg(bestSU));
    rPM = bestPM;
    rPA = bestPA;
    rSU = bestSU;
}

double RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE::SUa(int numParallellEvaluators, int numParticles)
{
    if (numParallellEvaluators > 0)
    {
        int maxParticlesPerEvaluator = numParticles / numParallellEvaluators;

        // If integer division has remanider, then ballancing is uneven, we need one addtional evaluation run (not all evaluatros will be used)
        if (numParticles % numParallellEvaluators != 0)
        {
            maxParticlesPerEvaluator++;
        }
        return double(numParticles) / double(maxParticlesPerEvaluator);
    }
    return 0;
}

double RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE::SUm(int nThreads)
{
    //! @todo calling it speed is bad, its the simtime lower is better (evaluation better)
    if (!mNumThreadsVsModelEvalTime.isEmpty())
    {
        double baseEvalTime = mNumThreadsVsModelEvalTime.first();
        if (nThreads <= mNumThreadsVsModelEvalTime.size())
        {
            return baseEvalTime / mNumThreadsVsModelEvalTime[nThreads-1];
        }
        //! @todo what if nThreads to high, right now we pretend no speedup, maybe should return negative value
    }

    return 0;
}


double RemoteModelSimulationQueuer_CRFP1_HOMO_RESCHEDULE::SUa(int numParallellEvaluators, int numParticles)
{
    // Need to lookup from table, approximate speedup for algorithms, absed on method
    QVector<double> sua {1.0, 1.43, 1.78, 1.81, 1.87, 1.99, 1.91, 1.92};
    if ( (numParallellEvaluators > 0) && (numParallellEvaluators <= sua.size()) )
    {
        return sua[numParallellEvaluators-1];
    }

    return 0.;
}


double RemoteModelSimulationQueuer_CRFP2_HOMO_RESCHEDULE::SUa(int numParallellEvaluators, int numParticles)
{
    // Need to lookup from table, approximate speedup for algorithms, absed on method
    QVector<double> sua {1.0, 1.35, 1.52, 1.58, 1.72, 1.74, 1.78, 1.79};

    if ( (numParallellEvaluators > 0 ) && (numParallellEvaluators <= sua.size()) )
    {
        return sua[numParallellEvaluators-1];
    }

    return 0.;
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
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE();
    }
    else if (type == Crfp1_Homo_Reschedule)
    {
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer_CRFP1_HOMO_RESCHEDULE();
    }
    else if (type == Crfp2_Homo_Reschedule)
    {
        gpRemoteModelSimulationQueuer = new RemoteModelSimulationQueuer_CRFP2_HOMO_RESCHEDULE();
    }
}

RemoteModelSimulationQueuer *gpRemoteModelSimulationQueuer=0;

#include "RemoteSimulationUtils.moc"

#endif
