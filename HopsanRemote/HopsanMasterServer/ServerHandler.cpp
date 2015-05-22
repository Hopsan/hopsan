#include "ServerHandler.h"
#include "common.h"
#include "RemoteHopsanClient.h"

#include "zmq.hpp"

#include <iostream>
#include <climits>
#include <thread>
#include <fstream>
#include <sstream>

using namespace std;
using namespace std::chrono;

extern zmq::context_t gContext;

template <typename T>
bool listContains(const std::list<T> &rList, const T &rValue)
{
    for (const T &v : rList)
    {
        if (v == rValue)
        {
            return true;
        }
    }
    return false;
}

bool sameIP(const ServerInfo &rSI1, const ServerInfo &rSI2)
{
    return (rSI1.ip == rSI2.ip) && (rSI1.port == rSI2.port);
}

ServerInfo ServerHandler::getServerNoLock(int id)
{
    auto it = mServerMap.find(id);
    if (it != mServerMap.end())
    {
        return it->second;
    }
    else
    {
        return ServerInfo();
    }
}

void ServerHandler::consistent()
{
    int d = int(mServerMap.size()) - int(mServerAgeList.size()) - int(mServerRefreshList.size());
    if (d != 0)
    {
        std::cout << "Error: Inconsisten num servers: " << d << std::endl;
    }
}

void ServerHandler::findNull()
{
    for (auto &p : mServerMap)
    {
        if (!p.second.isValid())
        {
            cout << "Error: A server is NULL" << endl;
        }
    }
}

void ServerHandler::addServer(ServerInfo &rServerInfo)
{
    std::lock_guard<std::mutex> lock(mMutex);
    // Default id value is 0, it will be applied to the first server added
    int id=0;
    // If we have free ids, than take one of them
    //! @todo should have a quarantine period befor pushing into free ids, to avoid nasty code taht handles what if server was replaced during the last milliseconds
    if (!mFreeIds.empty())
    {
        id = mFreeIds.front();
        mFreeIds.pop_front();
    }
    // Else we ask for the highest id in the map, and add one to that
    else if (!mServerMap.empty())
    {
        id = (mServerMap.rbegin())->first + 1;
    }
    // Now set the id in the server info opbject
    rServerInfo.mId = id;

    cout << PRINTSERVER << nowDateTime() << " Adding server: " << id << " IP: " << rServerInfo.ip << ":" << rServerInfo.port;
    if (!rServerInfo.description.empty())
    {
        cout << " (" << rServerInfo.description << ")";
    }
    cout << endl;

    // Insert into the map
    mServerMap.insert( {id, rServerInfo} );

    // Here we push to front, bacause new servers need emmediate update
    mServerAgeList.push_front(rServerInfo.mId);
}

void ServerHandler::updateServerInfoNoLock(const ServerInfo &rServerInfo)
{
    if (rServerInfo.isValid())
    {
        // Here we assume that serverId is in mServerRefreshList
        mServerMap.at(rServerInfo.mId) = rServerInfo;
        mServerRefreshList.remove(rServerInfo.mId);
        mServerAgeList.push_back(rServerInfo.mId);
    }
    else
    {
        cout << "Error: Trying to add inValid server in update" << endl;
    }
}

void ServerHandler::removeServer(int id)
{
    mMutex.lock();
    cout << PRINTSERVER << nowDateTime() << " Removing server: " << id << endl;
    // Remove the server info object and any occurance in the age and refresh lists
    mServerMap.erase(id);
    mServerAgeList.remove(id);
    mServerRefreshList.remove(id);
    // This id is now free to use by an other server
    mFreeIds.push_back(id);
    mMutex.unlock();
}

ServerInfo ServerHandler::getServer(int id)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return getServerNoLock(id);
}

int ServerHandler::getServerIDMatching(std::string ip, std::string port)
{
    std::lock_guard<std::mutex> lock(mMutex);
    for(auto &item : mServerMap )
    {
        if ( (item.second.ip == ip) && (item.second.port == port) )
        {
            return item.first;
        }
    }
    return -1;
}

ServerHandler::idlist_t ServerHandler::getServers(double maxTime, int minNumThreads, int maxNum)
{
    if (maxNum < 0)
    {
        maxNum = INT_MAX;
    }

    idlist_t ids;
    mMutex.lock();
    for (auto &item : mServerMap)
    {
        if (item.second.isReady && item.second.numTotalSlots > minNumThreads && item.second.benchmarkTime < maxTime)
        {
            ids.push_back(item.first);
            if (int(ids.size()) >= maxNum)
            {
                break;
            }
        }
    }
    mMutex.unlock();

    return ids;
}

void ServerHandler::getOldestServer(int &rID, std::chrono::steady_clock::time_point &rTime)
{
    mMutex.lock();
    if (mServerAgeList.empty())
    {
        rID = -1;
    }
    else
    {
        rID = mServerAgeList.front();
        rTime = mServerMap.at(rID).lastCheckTime;
    }
    mMutex.unlock();
}

int ServerHandler::getOldestServer()
{
    std::chrono::steady_clock::time_point dummy;
    int id;
    getOldestServer(id, dummy);
    return id;
}

void ServerHandler::refreshServerStatus(int serverId)
{
    mMutex.lock();
    if (listContains(mServerAgeList, serverId))
    {
        mServerRefreshList.push_back(serverId);
        mServerAgeList.remove(serverId);
        std::thread (&ServerHandler::refreshServerStatusThread, this, serverId).detach();
    }
    mMutex.unlock();
}

void ServerHandler::refreshServerBenchmark(int serverId)
{
    mMutex.lock();
    std::thread (&ServerHandler::refreshServerBenchmarkThread, this, serverId).detach();
    mMutex.unlock();
}

void ServerHandler::refreshServerStatusThread(int serverId)
{
    mNumRunningRefreshServerStatusThreads++;
    RemoteHopsanClient hopsanClient(gContext);
    if (hopsanClient.areSocketsValid())
    {
        ServerInfo server = getServer(serverId);
        if (server.isValid())
        {
            cout << PRINTSERVER << nowDateTime() << " Requesting status from server: " << serverId << endl;
            hopsanClient.connectToServer(server.ip, server.port);
            ServerStatusT status;
            bool rc = hopsanClient.requestServerStatus(status);
            if (rc)
            {
                mMutex.lock();
                cout << PRINTSERVER << nowDateTime() << " Server: " << serverId << " is responding!" << endl;
                // We need to get server again to verify that it has not been replaced while we were communicating
                ServerInfo server2 = getServerNoLock(serverId);
                if (sameIP(server, server2))
                {
                    server2.lastCheckTime = steady_clock::now();
                    server2.isReady = status.isReady;
                    server2.numTotalSlots = status.numTotalSlots;
                    updateServerInfoNoLock(server2);
                }
                mMutex.unlock();
            }
            else
            {
                cout << PRINTSERVER << nowDateTime() << " Server: " << serverId << " is NOT responding!" << endl;
                removeServer(serverId);
                //! @todo what if network temporarily down
            }
        }
    }
    mNumRunningRefreshServerStatusThreads--;
}

void ServerHandler::refreshServerBenchmarkThread(int serverId)
{
    mNumRunningRefreshServerStatusThreads++;
    RemoteHopsanClient hopsanClient(gContext);
    if (hopsanClient.areSocketsValid())
    {
        ServerInfo server = getServer(serverId);
        if (server.isValid())
        {
            cout << PRINTSERVER << nowDateTime() << " Requesting benchmark from server: " << serverId << endl;
            hopsanClient.connectToServer(server.ip, server.port);

            ifstream benchmarkmodel(BENCHMARKMODEL);
            if (!benchmarkmodel.is_open())
            {
                cout << PRINTSERVER << nowDateTime() << "Error: Could not open file " << BENCHMARKMODEL << endl;
            }

            std::stringstream filebuffer;
            filebuffer << benchmarkmodel.rdbuf();

            double benchmarkTime;
            size_t nThreads = 1; //! @todo not 1 should cycle and test all
            bool rc = hopsanClient.blockingBenchmark(filebuffer.str(), nThreads, benchmarkTime);
            if (rc)
            {
                mMutex.lock();
                cout << PRINTSERVER << nowDateTime() << " Got server: " << serverId << " benchmark" << endl;
                // We need to get server again to verify that it has not been replaced while we were communicating
                ServerInfo server2 = getServerNoLock(serverId);
                if (sameIP(server, server2))
                {
                    if (server2.benchmarkTimes.size() < nThreads)
                    {
                        server2.benchmarkTimes.resize(nThreads);
                    }
                    server2.benchmarkTimes[nThreads-1] = benchmarkTime;
                    server2.benchmarkTime = server2.benchmarkTimes.front();
                    updateServerInfoNoLock(server2);
                }
                mMutex.unlock();
            }
            else
            {
                cout << PRINTSERVER << nowDateTime() << " Server: " << serverId << " is NOT responding!" << endl;
            }
        }
    }
    mNumRunningRefreshServerStatusThreads--;
}

size_t ServerHandler::numServers()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mServerMap.size();
}
