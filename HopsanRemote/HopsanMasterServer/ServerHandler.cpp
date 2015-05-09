#include "ServerHandler.h"
#include "common.h"
#include "RemoteHopsanClient.h"

#include "zmq.hpp"

#include <iostream>
#include <climits>

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
    int d = int(mServerMap.size()) - int(mServerAgeList.size()) /*- int(mServerRefreshList.size())*/;
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

    // DEBUG
//    if (mServerMap.count(id) > 0)
//    {
//        cout << "Error: Trying to add server with id that already exist in map" << endl;
//    }
//    if (listContains(mServerAgeList, id))
//    {
//        cout << "Error: Trying to add server with id that already exist in age list" << endl;
//    }

    cout << PRINTSERVER << nowDateTime() << " Adding server: " << id << " IP: " << rServerInfo.ip << " Port: " << rServerInfo.port << endl;

    // Insert into the map
    mServerMap.insert( std::make_pair(id, rServerInfo) );

    // Here we push to front, bacause new servers need emmediate update
    mServerAgeList.push_front(rServerInfo.mId);
}

void ServerHandler::updateServerInfoNoLock(const ServerInfo &rServerInfo)
{
    if (rServerInfo.isValid())
    {
        // We only allow re insert if server was actually in refresh list, (to avoid re inserting it if it was just remove during refresh)
        //if (listContains(mServerRefreshList, rServerInfo.mId))
        {
            mServerMap.at(rServerInfo.mId) = rServerInfo;
        //    mServerRefreshList.remove(rServerInfo.mId); //! @todo is this list even needed
            mServerAgeList.remove(rServerInfo.mId);
            mServerAgeList.push_back(rServerInfo.mId);
        }
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
    //mServerRefreshList.remove(id);
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

//steady_clock::time_point ServerHandler::getServerAge(int id)
//{
//    //! @todo should have smarter mutex solution to allow multiple reads as long as there is no write
//    std::lock_guard<std::mutex> lock(mMutex);
//    std::chrono::steady_clock::time_point tp;
//    try
//    {
//        tp = mServerMap.at(id).lastCheckTime;
//    }
//    catch(std::exception e)
//    {
//        cout << "FUCK YOU: " << e.what() << endl;
//    }
//    return tp;
//}

ServerHandler::idlist_t ServerHandler::getServersFasterThen(double maxTime, int maxNum)
{
    if (maxNum < 0)
    {
        maxNum = INT_MAX;
    }

    idlist_t ids;
    mMutex.lock();
    for (auto &item : mServerMap)
    {
        if (item.second.isReady && item.second.benchmarkTime < maxTime)
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

void ServerHandler::refreshServerStatus(size_t serverId)
{

}

void ServerHandler::refreshServerStatusThread(size_t serverId)
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
            bool rc = hopsanClient.requestStatus(status);
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

size_t ServerHandler::numServers()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mServerMap.size();
}

//ServerHandler::idlist_t ServerHandler::getServersToRefresh(double maxAge, int maxNumServers)
//{
//    if (maxNumServers < 0)
//    {
//        maxNumServers = INT_MAX;
//    }

//    std::list<size_t> ids;
//    mMutex.lock();
//    for(auto &server : mServerMap)
//    {
//        duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - server.second.lastCheckTime);
//        if (time_span.count() > maxAge)
//        {
//            ids.push_back(server.second.id());
//            if (int(ids.size()) >= maxNumServers)
//            {
//                // Break loop if we have collected enough servers
//                break;
//            }
//        }
//    }
//    mMutex.unlock();
//    return ids;
//}

//ServerHandler::idlist_t ServerHandler::getOldestServers(size_t maxNum)
//{
//    idlist_t results;
//    mMutex.lock();
//    cout << "mServerAgeList.size(): " << mServerAgeList.size() << endl;
//    cout << "mServerRefreshList.size(): " << mServerRefreshList.size() << endl;
//    for (size_t i=0; i<min(maxNum, mServerAgeList.size()); ++i)
//    {
//        results.push_back(mServerAgeList.front());
//        mServerRefreshList.push_back(mServerAgeList.front());
//        mServerAgeList.pop_front();
//        findNull();
//        consistent();
//    }
//    mMutex.unlock();
//    return results;
//}
