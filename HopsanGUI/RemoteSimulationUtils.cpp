#include "RemoteSimulationUtils.h"
#include "Configuration.h"
#include "global.h"
#include "Widgets/ModelWidget.h"

#ifdef USEZMQ

#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QEventLoop>

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

void RemoteModelSimulationQueuer::setupSimulationHandlers(QVector<ModelWidget *> models)
{
    clear();
    mAllModels = models;

    // Get all servers
    if(mpRemoteCoreAddressHandler.isNull())
    {
        mpRemoteCoreAddressHandler = getSharedRemoteCoreAddressHandler();
    }

    // Disconnect if address has changed
    if (mpRemoteCoreAddressHandler->getAddressAndPort() != gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESSSERVERADDRESS))
    {
        mpRemoteCoreAddressHandler->disconnect();
    }

    bool addrserver_connected = mpRemoteCoreAddressHandler->isConnected();
    if (!addrserver_connected)
    {
        mpRemoteCoreAddressHandler->setHopsanAddressServer(gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESSSERVERADDRESS));
        addrserver_connected = mpRemoteCoreAddressHandler->connect();
    }

    if (addrserver_connected)
    {
        mpRemoteCoreAddressHandler->requestAvailableServers();
        QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(1e200, mnThreadsPerModel);

        if (!servers.isEmpty())
        {
            int nServers = 0;
            int enqueCtr = 0;
            for (int m=0; m<models.size(); ++m)
            {
                QList<QString> servers = mpRemoteCoreAddressHandler->getMatchingAvailableServers(1e200, mnThreadsPerModel);
                ModelWidget *pModel = models[m];

                // If we run out of servers, then enque models
                if (servers.isEmpty())
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
                else
                {
                    QString server = servers.front();
                    QStringList sserver = server.split(":");
                    servers.pop_front();

                    SharedRemoteCoreSimulationHandlerT pSH(new RemoteCoreSimulationHandler());
                    pSH->setHopsanServer(sserver.first(), sserver.last());
                    pSH->setNumThreads(mnThreadsPerModel);
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
        }
    }
}

void RemoteModelSimulationQueuer::simulateModels()
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

    // Create a copy since we weill deque and pop pointers
    QVector<QQueue<ModelWidget*>> modelQueues = mModelQueues;

    int prev_m=0;
    for (int m=0; m<mAllModels.size(); ++m)
    {
        // Block if until next element in queue can be run
        if (barriers[h]->tryLock())
        {
            ModelWidget *pModel = modelQueues[h].dequeue();
            SharedRemoteCoreSimulationHandlerT pRCSH = mRemoteCoreSimulationHandlers[h];

            pModel->setUseRemoteSimulationCore(pRCSH);
            bool rc = pModel->loadModelRemote();
            QObject::connect(pModel, SIGNAL(simulationFinished()), barriers[h], SLOT(unlock()));
            pModel->simulate_nonblocking();

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

void RemoteModelSimulationQueuer::clear()
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

RemoteModelSimulationQueuer gRemoteModelSimulationQueuer;


#include "RemoteSimulationUtils.moc"

#endif

