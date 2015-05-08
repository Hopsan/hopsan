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

void ServerHandler::consistent()
{
    int d = int(mServerMap.size()) - int(mServerAgeList.size()) - int(mServerRefreshList.size());
    if (d != 0)
    {
        std::cout << "Error: Inconsisten num servers: " << d << std::endl;
    }
}

size_t ServerHandler::addServer(ServerInfo &rServerInfo)
{
    std::lock_guard<std::mutex> lock(mMutex);
    int id=0;
    if (!mFreeIds.empty())
    {
        id = mFreeIds.front();
        mFreeIds.pop_front();
    }
    else if (!mServerMap.empty())
    {
        id = (mServerMap.rbegin())->first;
        id = id+1;
    }
    rServerInfo.mId = id;

    if (mServerMap.count(id) > 0)
    {
        cout << "Error: Trying to add server with id that already exist in map" << endl;
    }

    if (listContains(mServerAgeList, id))
    {
        cout << "Error: Trying to add server with id that already exist in age list" << endl;
    }

    cout << PRINTSERVER << nowDateTime() << " Adding server: " << id << " IP: " << rServerInfo.ip << " Port: " << rServerInfo.port << endl;

    if (rServerInfo.isValid())
    {
        mServerMap.insert( std::make_pair(id, rServerInfo) );

        if (!mServerMap[id].isValid())
        {
            cout << "Error: Add inValid server in addServer" << endl;
        }

        mServerAgeList.push_front(rServerInfo.mId); // Here we push to front, bacause new servers need emmediate update
        consistent();
    }
    else
    {
        cout << "Error: Trying to add inValid server in addServer" << endl;
    }
    return rServerInfo.mId;
}

void ServerHandler::updateServerInfo(ServerInfo &rServerInfo)
{
    mMutex.lock();
    if (rServerInfo.isValid())
    {
        // We only allow re insert if server was actually in refresh list, (to avoid re inserting it if it was just remove during refresh)
        if (listContains(mServerRefreshList, rServerInfo.mId))
        {
            mServerMap[rServerInfo.mId] = rServerInfo;
            mServerRefreshList.remove(rServerInfo.mId); //! @todo is this list even needed
            mServerAgeList.push_back(rServerInfo.mId);
        }
        consistent();
    }
    else
    {
        cout << "Error: Trying to add inValid server in update" << endl;
    }
    mMutex.unlock();
}

void ServerHandler::removeServer(int id)
{
    mMutex.lock();
    cout << PRINTSERVER << nowDateTime() << " Removing server: " << id << endl;
    mServerMap.erase(id);
    mServerAgeList.remove(id);
    mServerRefreshList.remove(id);
    mFreeIds.push_back(id);
    consistent();
    mMutex.unlock();
}

ServerInfo ServerHandler::getServer(int id)
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

steady_clock::time_point ServerHandler::getServerAge(int id)
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

ServerHandler::idlist_t ServerHandler::getOldestServers(size_t maxNum)
{
    idlist_t results;
    mMutex.lock();
    cout << "mServerAgeList.size(): " << mServerAgeList.size() << endl;
    cout << "mServerRefreshList.size(): " << mServerRefreshList.size() << endl;
    for (size_t i=0; i<min(maxNum, mServerAgeList.size()); ++i)
    {
        results.push_back(mServerAgeList.front());
        mServerRefreshList.push_back(mServerAgeList.front());
        mServerAgeList.pop_front();
    }
    consistent();
    mMutex.unlock();
    return results;
}
