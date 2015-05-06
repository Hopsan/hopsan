#include "ServerHandler.h"
#include "common.h"

#include <iostream>
#include <climits>

using namespace std;
using namespace std::chrono;

void ServerHandler::addServer(ServerInfo server)
{
    mLock.lock();
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
    mLock.unlock();
}

void ServerHandler::updateServerInfo(ServerInfo server)
{
    mLock.lock();
    mServerMap[server.id()] = server;
    mLock.unlock();
}

void ServerHandler::removeServer(size_t id)
{
    mLock.lock();
    cout << PRINTSERVER << nowDateTime() << " Removing server: " << id << endl;
    mServerMap.erase(id);
    mFreeIds.push_back(id);
    mLock.unlock();
}

ServerInfo ServerHandler::getServer(size_t id)
{
    ServerInfo si;
    mLock.lock();
    auto it = mServerMap.find(id);
    if (it != mServerMap.end())
    {
        si = it->second;
    }
    mLock.unlock();
    return si;
}

int ServerHandler::getServerMatching(std::string ip, std::string port)
{
    for(auto &item : mServerMap )
    {
        if ( (item.second.ip == ip) && (item.second.port == port) )
        {
            return item.first;
        }
    }
    return -1;
}

ServerHandler::idlist_t ServerHandler::getServersFasterThen(double maxTime, int maxNum)
{
    if (maxNum < 0)
    {
        maxNum = INT_MAX;
    }

    idlist_t ids;
    mLock.lock();
    for (auto &item : mServerMap)
    {
        if (item.second.isReady && item.second.benchmarkTime < maxTime)
        {
            ids.push_back(item.first);
            if (ids.size() >= maxNum)
            {
                break;
            }
        }
    }
    mLock.unlock();

    return ids;
}

size_t ServerHandler::numServers()
{
    return mServerMap.size();
}

ServerHandler::idlist_t ServerHandler::getServersToRefresh(double maxAge)
{
    std::list<size_t> ids;
    for(auto &server : mServerMap)
    {
        duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - server.second.lastCheckTime);
        if (time_span.count() > maxAge)
        {
            ids.push_back(server.second.id());
        }
    }
    return ids;
}
