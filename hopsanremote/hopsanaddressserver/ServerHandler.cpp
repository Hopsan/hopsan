/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#include "ServerHandler.h"
#include "common.h"
#include "hopsanremoteclient/RemoteHopsanClient.h"
#include "hopsanremotecommon/MessageUtilities.h"

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
    return (rSI1.address == rSI2.address);
}

//std::list<std::string> splitstring(const std::string &str, const std::string &delim)
//{
//    std::list<std::string> out;
//    size_t b=0, e, n;
//    bool exit=false;
//    do
//    {
//        e = str.find_first_of(delim, b);
//        if (e == std::string::npos)
//        {
//            n = e;
//            exit=true;
//        }
//        else
//        {
//            n = e-b;
//        }
//        out.push_back(str.substr(b,n));
//        b = e+1;
//    }while(!exit);
//    return out;
//}

bool matchSubnet(const std::string &ip, const std::string &subnetmatch)
{
    std::vector<std::string> ipfields, matchfields;

    // IPV4
    if (ip.find_first_of(".") != std::string::npos)
    {
        ipfields = splitstring(ip, ".");
        matchfields = splitstring(subnetmatch, ".");

        if (!ipfields.empty() && ipfields.size() == matchfields.size())
        {
            auto ipit = ipfields.begin();
            auto mait = matchfields.begin();
            while (ipit != ipfields.end())
            {
                // Check if not match
                if ((*ipit != *mait) && *mait != "*")
                {
                    return false;
                }

                ++ipit;
                ++mait;
            }
            // If we get here we have a match
            return true;
        }
    }

    //! @todo IPV6
    return false;
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
    // If we have servers them ask for the highest id in the map, and add one to that
    if (!mServerMap.empty())
    {
        id = (mServerMap.rbegin())->first + 1;
    }
    // Now set the id in the server info opbject
    rServerInfo.mInternalId = id;

    // Figure out if this is a subnet computer and what relay range it should have
    std::string srvip,srvport,srvrelay;
    splitaddress(rServerInfo.address, srvip, srvport, srvrelay);
    if (matchSubnet(srvip, gSubnetMatch))
    {
        int relayBaseId = reserveRelayBaseIdentityNoLock();
        if (relayBaseId != 0)
        {
            rServerInfo.mRelayBaseIdentity = to_string(relayBaseId);
        }
        else
        {
            cout << PRINTSERVER << nowDateTime() << " Error: Failed to reserv relay identities" << endl;
        }
    }

    cout << PRINTSERVER << nowDateTime() << " Adding server: " << id << " Address: " << rServerInfo.address;
    if (!rServerInfo.description.empty())
    {
        cout << " (" << rServerInfo.description << ")";
    }
    cout << endl;

    // Insert into the map
    mServerMap.insert( {id, rServerInfo} );

    // Here we push to front, because new servers need immediate update
    mServerAgeList.push_front(rServerInfo.mInternalId);
}

void ServerHandler::updateServerInfoNoLock(const ServerInfo &rServerInfo)
{
    if (rServerInfo.isValid())
    {
        // Here we assume that serverId is in mServerRefreshList
        mServerMap.at(rServerInfo.mInternalId) = rServerInfo;
        mServerRefreshList.remove(rServerInfo.mInternalId);
        mServerAgeList.push_back(rServerInfo.mInternalId);
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
    // Remove the server info object and any occurrence in the age and refresh lists

    //! @todo how to clear relays if we get here and has not clear them outside, could have timestamps in relays and purge them if not used for long

    mServerMap.erase(id);
    mServerAgeList.remove(id);
    mServerRefreshList.remove(id);
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
        if ( item.second.address == ip+":"+port )
        {
            return item.first;
        }
    }
    return -1;
}

Relay *ServerHandler::createNewRelay(const string &rRelayBaseIdentiy, int port)
{
    std::lock_guard<std::mutex> lock(mMutex);
    for(auto &item : mServerMap )
    {
        ServerInfo &si = item.second;
        if ( si.mRelayBaseIdentity == rRelayBaseIdentiy )
        {
            int subrelayid = si.mRelaySubIdentities.addOne();

            std::string newrelayid = rRelayBaseIdentiy+"."+to_string(subrelayid);
            std::string srvip, srvport, srvrelid;
            splitaddress(si.address, srvip, srvport, srvrelid);

            // Port -1 means use server port
            if (port < 0)
            {
                port = atoi(srvport.c_str());
            }
            return gRelayHandler.addRelay(newrelayid, makeZMQAddress(srvip, port));
        }
    }
    return nullptr;
}

void ServerHandler::removeRelay(const string &rRelayIdentiy)
{
    std::lock_guard<std::mutex> lock(mMutex);
    vector<string> fields = splitstring(rRelayIdentiy, ".");
    if (fields.size() == 2)
    {
        for(auto &item : mServerMap )
        {
            ServerInfo &si = item.second;
            if ( si.mRelayBaseIdentity == fields.front() )
            {
                si.mRelaySubIdentities.remove(atoi(fields.back().c_str()));
            }
        }
    }
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
    //if (hopsanClient.areSocketsValid())
    {
        ServerInfo server = getServer(serverId);
        if (server.isValid())
        {
            cout << PRINTSERVER << nowDateTime() << " Requesting status from server: " << serverId << endl;
            hopsanClient.connectToServer(server.address);
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
    //if (hopsanClient.areSocketsValid())
    {
        ServerInfo server = getServer(serverId);
        if (server.isValid())
        {
            cout << PRINTSERVER << nowDateTime() << " Requesting benchmark from server: " << serverId << endl;
            hopsanClient.connectToServer(server.address);

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
                cout << PRINTSERVER << nowDateTime() << " Benchmarking Server: " << serverId << " is NOT responding!" << endl;
            }
        }
    }
    mNumRunningRefreshServerStatusThreads--;
}

int ServerHandler::reserveRelayBaseIdentityNoLock()
{
    // For simplicity always reserve from the end
    //! @todo maybe use indexintervalcollection here instead
    int max = 0;
    if (!mRelayBaseIdentites.empty())
    {
        max = *mRelayBaseIdentites.rbegin();
    }
    // This will start at max+1  which means that if the set is empty the first value will be 1
    // and that is correct, 0 is not allowed as a relay identity
    max++;
    mRelayBaseIdentites.insert(max);
    return max;
}

void ServerHandler::unreserveRelayIdentitiesNoLock(ServerHandler::idlist_t ids)
{
    //std::lock_guard<std::mutex> lock(mMutex);
    for (auto it = ids.begin(); it != ids.end(); ++it)
    {
        mRelayBaseIdentites.erase(*it);
    }
}

size_t ServerHandler::numServers()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mServerMap.size();
}

size_t ServerHandler::numServersInAgeList()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mServerAgeList.size();
}
