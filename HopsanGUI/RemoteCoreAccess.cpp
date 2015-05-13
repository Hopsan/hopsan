//!
//! @file   RemoteCoreAccess.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-05-11
//!
//! @brief Contains the HopsanCore Qt API classes for communication with the Remote HopsanCore
//!
//$Id$

#include "RemoteCoreAccess.h"

#include <QFile>
#include <QVector>
#include <QTextStream>


#ifdef USEZMQ

// RemoteServerClient includes
#include "RemoteHopsanClient.h"
#ifdef _WIN32
zmq::context_t zmqContext(1, 63);
#else
zmq::context_t zmqContext(1);
#endif

// This variable must be instansiated AFTER zmqContext so that they are destoryed in the correct order
// It must be in this file so that we can be sure of that
SharedRemoteCoreAddressHandlerT gpRemoteCoreAddressHandler;

SharedRemoteCoreAddressHandlerT getSharedRemoteCoreAddressHandler()
{
    if (gpRemoteCoreAddressHandler.isNull())
    {
        gpRemoteCoreAddressHandler = SharedRemoteCoreAddressHandlerT(new RemoteCoreAddressHandler());
    }
    return gpRemoteCoreAddressHandler;
}


RemoteCoreSimulationHandler::RemoteCoreSimulationHandler()
{
    mpRemoteHopsanClient = new RemoteHopsanClient(zmqContext);
}


RemoteCoreSimulationHandler::~RemoteCoreSimulationHandler()
{
    if (mpRemoteHopsanClient->workerConnected() || mpRemoteHopsanClient->serverConnected())
    {
        disconnect();
    }
    if (mpRemoteHopsanClient)
    {
        delete mpRemoteHopsanClient;
        mpRemoteHopsanClient = 0;
    }
}

void RemoteCoreSimulationHandler::setHopsanServer(QString ip, QString port)
{
    mRemoteServerAddress = ip;
    mRemoteServerPort = port;
}

void RemoteCoreSimulationHandler::setNumThreads(int nThreads)
{
    mNumThreads = nThreads;
}

int RemoteCoreSimulationHandler::numThreads() const
{
    return mNumThreads;
}


bool RemoteCoreSimulationHandler::connect()
{
    if (!mRemoteServerAddress.isEmpty() && !mRemoteServerPort.isEmpty())
    {
        mpRemoteHopsanClient->connectToServer(mRemoteServerAddress.toStdString(), mRemoteServerPort.toStdString());
        if (mpRemoteHopsanClient->serverConnected())
        {
            size_t workerPort;
            if (mpRemoteHopsanClient->requestSlot(mNumThreads, workerPort))
            {
                mpRemoteHopsanClient->connectToWorker(mRemoteServerAddress.toStdString(), QString("%1").arg(workerPort).toStdString());
                if (mpRemoteHopsanClient->workerConnected())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void RemoteCoreSimulationHandler::disconnect()
{
    mpRemoteHopsanClient->disconnect();
}

bool RemoteCoreSimulationHandler::loadModel(QString hmfModelFile)
{
    QFile hmfFile(hmfModelFile);
    if (hmfFile.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&hmfFile);
        bool rc = mpRemoteHopsanClient->sendModelMessage(ts.readAll().toStdString());
        hmfFile.close();
        return rc;
    }
    else
    {
        //! @todo must report failed to open
        return false;
    }
}

bool RemoteCoreSimulationHandler::loadModelStr(QString hmfStr)
{
    if (!hmfStr.isEmpty())
    {
        return mpRemoteHopsanClient->sendModelMessage(hmfStr.toStdString());
    }
    return false;
}

bool RemoteCoreSimulationHandler::simulateModel()
{
    return mpRemoteHopsanClient->sendSimulateMessage(-1, -1, -1, -1, -1);
}

bool RemoteCoreSimulationHandler::getCoreMessages(QVector<QString> &rTypes, QVector<QString> &rTags, QVector<QString> &rMessages, bool includeDebug)
{
    std::vector<char> types;
    std::vector<std::string> tags, messages;
    bool rc = mpRemoteHopsanClient->requestMessages(types, tags, messages);
    if (rc)
    {
        // Here we copy messages AGAIN
        rTypes.resize(messages.size());
        rTags.resize(messages.size());
        rMessages.resize(messages.size());
        for (size_t m=0; m<messages.size(); ++m)
        {
            if (types[m] == 'f')
            {
                rTypes[m] = "fatal";
            }
            else if (types[m] == 'e')
            {
                rTypes[m] = "error";
            }
            else if (types[m] == 'w')
            {
                rTypes[m] = "warning";
            }
            else if (types[m] == 'i')
            {
                rTypes[m] = "info";
            }
            else if (types[m] == 'd')
            {
                if (includeDebug)
                {
                    rTypes[m] = "debug";
                }
                else
                {
                    //! @todo This will realocate message vectors not very clever, should use reserve and push_back instead maybe
                    rTypes.pop_back();
                    rMessages.pop_back();
                    rTags.pop_back();
                    continue;
                }
            }

            rTags[m] = QString::fromStdString(tags[m]);
            rMessages[m] = QString::fromStdString(messages[m]);
        }
        return true;
    }

    return false;
}

bool RemoteCoreSimulationHandler::getLogData(std::vector<std::string> *pNames, std::vector<double> *pData)
{
    return mpRemoteHopsanClient->requestSimulationResults(pNames, pData);
}

QString RemoteCoreSimulationHandler::getLastError() const
{
    return QString::fromStdString(mpRemoteHopsanClient->getLastErrorMessage());
}

void RemoteCoreAddressHandler::requestServerInfo(QString address)
{
    QStringList ad = address.split(":");
    // Here we create a new temporary HopsanClient to communicate with the HopsanServer
    RemoteHopsanClient client(zmqContext);
    client.connectToServer(ad.first().toStdString(), ad.last().toStdString());
    if (client.serverConnected())
    {
        ServerStatusT status;
        if (client.requestStatus(status))
        {
            // If we got status then update our mapped info, we use iterator and reference to avoid
            // inserting into map and therby destorying iterators that might be used when this function is called
            auto it = mAvailableServers.find(address);
            if (it != mAvailableServers.end())
            {
                it.value().nSlots = status.numTotalSlots;
                it.value().nOpenSlots = status.numFreeSlots;
                it.value().mResponding = true;
                //it.value().speed = status.
            }
        }
        //else server is dead maybe? Then discard the server entery
        else
        {
            // Cant remove here will break iterators in calling functions
            // Tagg it instead and schedule for later removal
            auto it = mAvailableServers.find(address);
            if (it != mAvailableServers.end())
            {
                it.value().mResponding = false;
                mNotRespondingServers.append(address);
                //it.value().speed = status.
            }
        }
        client.disconnect();
    }
}

void RemoteCoreAddressHandler::removeNotRespondingServers()
{
    while (!mNotRespondingServers.empty())
    {
        QString addr = mNotRespondingServers.front(); mNotRespondingServers.pop_front();
        auto it = mAvailableServers.find(addr);
        if (it != mAvailableServers.end())
        {
            double speed = it.value().speed;
            mAvailableServers.erase(it);
            mServerSpeedMap.remove(speed, addr); //! @todo since speed is double maybe we can not remove here, speed maybe should be int (ms)
        }
    }
}

RemoteCoreAddressHandler::RemoteCoreAddressHandler()
{
    mpRemoteHopsanClient = new RemoteHopsanClient(zmqContext);
}

RemoteCoreAddressHandler::~RemoteCoreAddressHandler()
{
    if (mpRemoteHopsanClient->serverConnected())
    {
        disconnect();
    }
    if (mpRemoteHopsanClient)
    {
        delete mpRemoteHopsanClient;
        mpRemoteHopsanClient = 0;
    }
}

void RemoteCoreAddressHandler::setHopsanAddressServer(QString ip, QString port)
{
    mHopsanAddressServerIP = ip;
    mHopsanAddressServerPort = port;

}

void RemoteCoreAddressHandler::setHopsanAddressServer(QString ip_port)
{
    QStringList fields = ip_port.split(":");
    if (fields.size() == 2)
    {
        setHopsanAddressServer(fields.first(), fields.last());
    }
    else
    {
        setHopsanAddressServer(fields.first(), mHopsanAddressServerPort);
    }
}

QString RemoteCoreAddressHandler::getAddressAndPort() const
{
    return mHopsanAddressServerIP+":"+mHopsanAddressServerPort;
}

bool RemoteCoreAddressHandler::isConnected()
{
    return mpRemoteHopsanClient->serverConnected();
}

bool RemoteCoreAddressHandler::connect()
{
    if (!mHopsanAddressServerIP.isEmpty() && !mHopsanAddressServerPort.isEmpty())
    {
        mpRemoteHopsanClient->connectToServer(mHopsanAddressServerIP.toStdString(), mHopsanAddressServerPort.toStdString());
        if (mpRemoteHopsanClient->serverConnected())
        {
            return true;
        }
    }
    return false;
}

void RemoteCoreAddressHandler::disconnect()
{
    mpRemoteHopsanClient->disconnect();
}

QList<QString> RemoteCoreAddressHandler::requestAvailableServers()
{
    //! @todo maybe should have a timer to prevent requesting multiple time within the same period
    mAvailableServers.clear();
    mServerSpeedMap.clear();
    if (mpRemoteHopsanClient->serverConnected())
    {
        std::vector<std::string> ips, ports;
        std::vector<int> numSlots;
        std::vector<double> speeds;
        mpRemoteHopsanClient->requestServerMachines(-1, 1e200, ips, ports, numSlots, speeds);
        for (size_t i=0; i<ips.size(); ++i)
        {
            //! @todo need common function for this add/update
            QString addr = QString("%1:%2").arg(ips[i].c_str()).arg(ports[i].c_str());
            ServerInfoT info;
            info.addr = addr;
            info.nSlots = numSlots[i];
            info.speed = speeds[i];
            mAvailableServers.insert(addr, info);
            mServerSpeedMap.insertMulti(info.speed, addr );
        }
    }
    return mAvailableServers.keys();
}

//QList<QString> RemoteCoreAddressHandler::requestAvailableServers(int nOpenSlots)
//{
//    requestAvailableServers();

//}

QString RemoteCoreAddressHandler::getBestAvailableServer(int nRequiredSlots)
{
    for (auto sit=mServerSpeedMap.begin(); sit!=mServerSpeedMap.end(); ++sit)
    {
        auto ait = mAvailableServers.find(sit.value());
        if (ait != mAvailableServers.end())
        {
            //! @todo here we would like to know about all open slots, but requesting every time may take time
            if (ait.value().nSlots >= nRequiredSlots )
            {
                requestServerInfo(ait.key()); //!  @todo should have last refresh time to avoid calling every time
                if (ait.value().mResponding && ait.value().nOpenSlots >= nRequiredSlots)
                {
                    return ait.value().addr;
                }
            }
        }
    }

    removeNotRespondingServers();

    // If we get here then everyone is taken, lets search again for the first one with an open slot
    for (auto sit=mServerSpeedMap.begin(); sit!=mServerSpeedMap.end(); ++sit)
    {
        auto ait = mAvailableServers.find(sit.value());
        if (ait != mAvailableServers.end())
        {
            //! @todo need a "request and reserver" function
            requestServerInfo(ait.key()); //!  @todo should have last refresh time to avoid calling every time
            if (ait.value().mResponding && ait.value().nOpenSlots > nRequiredSlots)
            {
                return ait.value().addr;
            }
        }
    }

    return "";
}

QList<QString> RemoteCoreAddressHandler::getMatchingAvailableServers(double requiredSpeed, int nRequiredSlots)
{
    QList<QString> results;
    for (auto sit=mServerSpeedMap.begin(); sit!=mServerSpeedMap.end(); ++sit)
    {
        if (sit.key() <= requiredSpeed)
        {
            auto ait = mAvailableServers.find(sit.value());
            if (ait != mAvailableServers.end())
            {
                if (ait.value().nSlots >= nRequiredSlots )
                {
                    requestServerInfo(ait.key()); //!  @todo should have last refresh time to avoid calling every time
                    if (ait.value().mResponding && ait.value().nOpenSlots >= nRequiredSlots)
                    {
                        results.append(ait.value().addr);
                    }
                }
            }
        }
    }

    removeNotRespondingServers();

    return results;
}

#endif

