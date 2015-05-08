#include "ServerHandler.h"
#include "common.h"

#include <iostream>
#include <climits>

using namespace std;
using namespace std::chrono;

template <typename T>
bool listContains(const std::list<T> &rList, T &rValue)
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

void ServerHandler::addServer(ServerInfo server)
{
    mMutex.lock();
    size_t id=0;
    if (!mFreeIds.empty())
    {
        id = mFreeIds.front();
        mFreeIds.pop_front();
    }
    else if (!mServerMap.empty())
    {
        id = (mServerMap.end()--)->first;
    }
    server.mId = id;
    cout << PRINTSERVER << nowDateTime() << " Adding server: " << id << " IP: " << server.ip << " Port: " << server.port << endl;
    mServerMap.insert(std::pair<size_t, ServerInfo>(id,server));
    mServerAgeList.push_back(server.mId);
    mMutex.unlock();
}

void ServerHandler::updateServerInfo(ServerInfo server)
{
    mMutex.lock();
    // We only allow re insert if server was actually in refresh list, (to avoid re inserting it if it was just remove during refresh)
    if (listContains(mServerRefreshList, server.mId))
    {
        mServerMap[server.mId] = server;
        mServerRefreshList.remove(server.mId); //! @todo is this list even needed
        mServerAgeList.push_back(server.mId);
    }
    mMutex.unlock();
}

void ServerHandler::removeServer(size_t id)
{
    mMutex.lock();
    cout << PRINTSERVER << nowDateTime() << " Removing server: " << id << endl;
    mServerMap.erase(id);
    mServerAgeList.remove(id);
    mServerRefreshList.remove(id);
    mFreeIds.push_back(id);
    mMutex.unlock();
}

ServerInfo ServerHandler::getServer(size_t id)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mServerMap.find(id);
    if (it != mServerMap.end())
    {
        return it->second;
    }
    return ServerInfo();
}

int ServerHandler::getServerMatching(std::string ip, std::string port)
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

steady_clock::time_point ServerHandler::getServerAge(size_t id)
{
    //! @todo should have smarter mutex solution to allow multiple reads as long as there is no write
    std::lock_guard<std::mutex> lock(mMutex);
    return mServerMap[id].lastCheckTime;
}

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

size_t ServerHandler::numServers()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mServerMap.size();
}

ServerHandler::idlist_t ServerHandler::getServersToRefresh(double maxAge, int maxNumServers)
{
    if (maxNumServers < 0)
    {
        maxNumServers = INT_MAX;
    }

    std::list<size_t> ids;
    mMutex.lock();
    for(auto &server : mServerMap)
    {
        duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - server.second.lastCheckTime);
        if (time_span.count() > maxAge)
        {
            ids.push_back(server.second.id());
            if (int(ids.size()) >= maxNumServers)
            {
                // Break loop if we have collected enough servers
                break;
            }
        }
    }
    mMutex.unlock();
    return ids;
}

ServerHandler::idlist_t ServerHandler::getOldestServers(size_t maxNum)
{
    idlist_t results;
    mMutex.lock();
    for (size_t i=0; i<min(maxNum, mServerAgeList.size()); ++i)
    {
        results.push_back(mServerAgeList.front());
        mServerRefreshList.push_back(mServerAgeList.front());
        mServerAgeList.pop_front();
    }
    mMutex.unlock();
    return results;
}
