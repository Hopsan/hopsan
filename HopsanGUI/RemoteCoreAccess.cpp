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
#include <QStringList>
#include <limits>


#ifdef USEZMQ

// RemoteServerClient includes
#include "hopsanremoteclient/RemoteHopsanClient.h"
#include "zmq.hpp"
#ifdef _WIN32
zmq::context_t zmqContext(1, 63);
#else
zmq::context_t zmqContext(1);
#endif

// This variable must be instantiated AFTER zmqContext so that they are destroyed in the correct order
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
    mpRemoteHopsanClient->setMaxWorkerStatusRequestWaitTime(0.2); //!< @todo This is a temporary hack, need to set this from the outside and maybe not request so often
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

void RemoteCoreSimulationHandler::setUserIdentification(QString useridstring)
{
    mRemoteUserIdentification = useridstring;
}

void RemoteCoreSimulationHandler::setAddressServer(QString fullAddress)
{
    mRemoteAddressServerFullAddress = fullAddress;
}

void RemoteCoreSimulationHandler::setHopsanServer(QString ip, QString port)
{
    QString addr = ip+":"+port;
    setHopsanServer(addr);
}

void RemoteCoreSimulationHandler::setHopsanServer(QString fullAddress)
{
    mRemoteServerFullAddress = fullAddress;
}

QString RemoteCoreSimulationHandler::getHopsanServerAddress() const
{
    return mRemoteServerFullAddress;
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
    connectAddressServer();
    if (connectServer())
    {
        return connectWorker();
    }
    return false;
}

void RemoteCoreSimulationHandler::disconnect()
{
    mpRemoteHopsanClient->disconnect();
}

bool RemoteCoreSimulationHandler::connectAddressServer()
{
    if(!mRemoteAddressServerFullAddress.isEmpty())
    {
        mpRemoteHopsanClient->connectToAddressServer(mRemoteAddressServerFullAddress.toStdString());
        if (mpRemoteHopsanClient->serverConnected())
        {
            return true;
        }
    }
    return false;
}

bool RemoteCoreSimulationHandler::connectServer()
{
    if (!mRemoteServerFullAddress.isEmpty())
    {
        mpRemoteHopsanClient->connectToServer(mRemoteServerFullAddress.toStdString());
        if (mpRemoteHopsanClient->serverConnected())
        {
            return true;
        }
    }
    return false;
}

bool RemoteCoreSimulationHandler::connectWorker()
{
    if (mpRemoteHopsanClient->serverConnected())
    {
        QString name,password;
        QStringList parts = mRemoteUserIdentification.split(":");
        name = parts.first();
        if (parts.size() > 1)
        {
            password = parts.last();
        }

        int ctrlPort;
        if (mpRemoteHopsanClient->requestSlot(mNumThreads, ctrlPort, name.toStdString()))
        {
            mpRemoteHopsanClient->connectToWorker(ctrlPort);
            if (mpRemoteHopsanClient->workerConnected())
            {
                if (!mRemoteUserIdentification.isEmpty())
                {
                    mpRemoteHopsanClient->sendUserIdentification(name.toStdString(), password.toStdString());
                }
                return true;
            }
        }
    }
    return false;
}

void RemoteCoreSimulationHandler::disconnectAddressServer()
{
    mpRemoteHopsanClient->disconnectAddressServer();
}

void RemoteCoreSimulationHandler::disconnectServer()
{
    mpRemoteHopsanClient->disconnectServer();
}

void RemoteCoreSimulationHandler::disconnectWorker()
{
    mpRemoteHopsanClient->disconnectWorker();
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

bool RemoteCoreSimulationHandler::sendAsset(QString fullFilePath, QString relativeFilePath, double *pProgress)
{
    return mpRemoteHopsanClient->blockingSendFile(fullFilePath.toStdString(), relativeFilePath.toStdString(), pProgress);
}

bool RemoteCoreSimulationHandler::simulateModel_blocking(double *pProgress)
{
    return mpRemoteHopsanClient->blockingSimulation(-1, -1, -1, -1, -1, pProgress);
}

bool RemoteCoreSimulationHandler::simulateModel_nonblocking()
{
    return mpRemoteHopsanClient->sendSimulateMessage(-1, -1, -1, -1, -1);
}

bool RemoteCoreSimulationHandler::abortSimulation()
{
    return mpRemoteHopsanClient->abortSimulation();
}

bool RemoteCoreSimulationHandler::benchmarkModel_blocking(const QString &rModel, const int nThreads, double &rSimTime)
{
    return mpRemoteHopsanClient->blockingBenchmark(rModel.toStdString(), nThreads, rSimTime);
}

bool RemoteCoreSimulationHandler::requestSimulationProgress(double *pProgress)
{
    WorkerStatusT status;
    bool rc = mpRemoteHopsanClient->requestWorkerStatus(status);
    if (rc)
    {
        if (status.simulation_inprogress)
        {
            *pProgress = status.simulation_progress;
        }
        else if (status.simulation_finished)
        {
            //! @todo what if simulation exits before it is finished
            *pProgress = 1.;
        }
        else
        {
            *pProgress = -1.;
        }
    }
    else
    {
        *pProgress = -1.;
    }
    return rc;
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
                    //! @todo This will reallocate message vectors not very clever, should use reserve and push_back instead maybe
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

bool RemoteCoreSimulationHandler::getLogData(QVector<RemoteResultVariable> &rResultVariables)
{
    std::vector<ResultVariableT> results;
    bool rc = mpRemoteHopsanClient->requestSimulationResults(results);
    rResultVariables.clear();
    rResultVariables.reserve(results.size());
    for (ResultVariableT &r : results)
    {
        rResultVariables.push_back(RemoteResultVariable());
        rResultVariables.last().fullname = QString::fromStdString(r.name);
        rResultVariables.last().alias = QString::fromStdString(r.alias);
        rResultVariables.last().quantity = QString::fromStdString(r.quantity);
        rResultVariables.last().unit = QString::fromStdString(r.unit);
        rResultVariables.last().data.swap(r.data);
    }
    return rc;
}

QString RemoteCoreSimulationHandler::getLastError() const
{
    return QString::fromStdString(mpRemoteHopsanClient->getLastErrorMessage());
}

void RemoteCoreAddressHandler::requestServerInfo(QString address)
{
    // Here we create a new temporary HopsanClient to communicate with the HopsanServer
    RemoteHopsanClient client(zmqContext);
    client.connectToAddressServer(getAddressAndPort().toStdString());
    client.connectToServer(address.toStdString());
    if (client.serverConnected())
    {
        ServerStatusT status;
        if (client.requestServerStatus(status))
        {
            // If we got status then update our mapped info, we use iterator and reference to avoid
            // inserting into map and thereby destroying iterators that might be used when this function is called
            auto it = mAvailableServers.find(address);
            if (it != mAvailableServers.end())
            {
                it.value().nSlots = status.numTotalSlots;
                it.value().nOpenSlots = status.numFreeSlots;
                it.value().mResponding = true;
                //it.value().speed = status.
            }
        }
        //else server is dead maybe? Then discard the server entry
        else
        {
            // Cant remove here will break iterators in calling functions
            // Tag it instead and schedule for later removal
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
            double evalTime = it.value().evalTime;
            mAvailableServers.erase(it);
            mServerSpeedMap.remove(evalTime, addr); //! @todo since speed is double maybe we can not remove here, speed maybe should be int (ms)
        }
    }
}

RemoteCoreAddressHandler::RemoteCoreAddressHandler()
{
    mpRemoteHopsanClient = new RemoteHopsanClient(zmqContext);
}

RemoteCoreAddressHandler::~RemoteCoreAddressHandler()
{
    if (mpRemoteHopsanClient->addressServerConnected())
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
    return mpRemoteHopsanClient->addressServerConnected();
}

int RemoteCoreAddressHandler::numKnownServers() const
{
    return mAvailableServers.size();
}

bool RemoteCoreAddressHandler::connect()
{
    if (!mHopsanAddressServerIP.isEmpty() && !mHopsanAddressServerPort.isEmpty())
    {
        mpRemoteHopsanClient->connectToAddressServer(getAddressAndPort().toStdString());
        if (mpRemoteHopsanClient->addressServerConnected())
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
    if (mpRemoteHopsanClient->addressServerConnected())
    {
        std::vector<ServerMachineInfoT> machines;
        mpRemoteHopsanClient->requestServerMachines(-1, 1e200, machines);
        for (size_t i=0; i<machines.size(); ++i)
        {
            //! @todo need common function for this add/update
            QString addr = QString::fromStdString(machines[i].relayaddress);
            if (addr.isEmpty())
            {
                addr = QString::fromStdString(machines[i].address);
            }

            ServerInfoT info;
            info.addr = addr;
            info.nSlots = machines[i].numslots;
            info.evalTime = machines[i].evalTime;
            mAvailableServers.insert(addr, info);
            mServerSpeedMap.insertMulti(info.evalTime, addr );
        }
    }
    return mAvailableServers.keys();
}

//QList<QString> RemoteCoreAddressHandler::requestAvailableServers(int nOpenSlots)
//{
//    requestAvailableServers();

//}

QString RemoteCoreAddressHandler::getBestAvailableServer(int nRequiredSlots, const QStringList &rExcludeList)
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
                if (ait.value().mResponding && ait.value().nOpenSlots >= nRequiredSlots && !rExcludeList.contains(ait.value().addr))
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

QList<QString> RemoteCoreAddressHandler::getMatchingAvailableServers(double requiredSpeed, int nRequiredSlots, const QStringList &rExcludeList)
{
    if (requiredSpeed < 0)
    {
        requiredSpeed = std::numeric_limits<double>::max();
    }

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
                    if (ait.value().mResponding && ait.value().nOpenSlots >= nRequiredSlots && !rExcludeList.contains(ait.value().addr))
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

void RemoteCoreAddressHandler::getMaxNumSlots(int &rMaxNumSlots, int &rNumServers)
{
    int maxSlots=-1;
    int numServers=0;

    for (const auto &kv : mAvailableServers )
    {
        if (kv.nSlots > maxSlots)
        {
            maxSlots=kv.nSlots;
            numServers=1;
        }
        else if (kv.nSlots == maxSlots)
        {
            numServers++;
        }
    }

    //! @todo what if the server with max slots have no open slots or is not accepting jobs
    rMaxNumSlots = maxSlots;
    rNumServers = numServers;
}

#endif

