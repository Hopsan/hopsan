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

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <list>
#include <atomic>
#include <vector>
#include <set>

#include "RelayHandler.h"

class ServerHandler;

//! @todo this is no longer a range should replace with indexintervalcollection
class Range
{
public:
    Range() {}

    int addOne()
    {
        int num = max()+1;
        mSet.insert(num);
        return num;
    }

    void remove(int num)
    {
        for (auto it=mSet.begin(); it!=mSet.end(); ++it)
        {
            if (*it == num)
            {
                mSet.erase(it);
            }
        }
    }

    //! @todo this wont work if remove has been called in teh middle
    bool inRange(int val) const
    {
        if (!empty())
        {
            return (val >= min()) && (val <= max());
        }
        return false;
    }

    std::list<int> to_list() const
    {
        std::list<int> out;
        for(int i : mSet)
        {
            out.push_back(i);
        }
        return out;
    }


    bool empty() const
    {
        return mSet.empty();
    }

    int min() const
    {
        if (mSet.empty())
        {
            return 0;
        }
        return *mSet.begin();
    }

    int max() const
    {
        if (mSet.empty())
        {
            return 0;
        }
        return *mSet.rbegin();
    }

private:
    std::set<int> mSet;
};

class ServerInfo
{
    friend class ServerHandler;
private:
    int mInternalId=-1;

public:
    std::string address;
    std::string description;
    std::string mRelayBaseIdentity;
    int numTotalSlots = 0;
    double benchmarkTime=1e100;
    std::vector<double> benchmarkTimes;
    std::chrono::steady_clock::time_point lastCheckTime;
    bool bussyProcessing=false;
    bool isReady=false;
    Range mRelaySubIdentities;

    bool needsRelay() const
    {
        return !mRelayBaseIdentity.empty();
    }

    inline int internalId() const
    {
        return mInternalId;
    }

    inline bool isValid() const
    {
        return (mInternalId >= 0);
    }
};

#define BENCHMARKMODEL "../Models/Example Models/Load Sensing System.hmf"

class ServerHandler
{
private:
    typedef std::list<int> idlist_t;
    idlist_t mServerAgeList;
    idlist_t mServerRefreshList;
    std::set<int> mRelayBaseIdentites;
    std::map<int, ServerInfo> mServerMap;
    std::mutex mMutex;

    ServerInfo getServerNoLock(int id);
    void updateServerInfoNoLock(const ServerInfo &rServerInfo);

    void refreshServerStatusThread(int serverId);
    void refreshServerBenchmarkThread(int serverId);

    int reserveRelayBaseIdentityNoLock();
    void unreserveRelayIdentitiesNoLock(idlist_t ids);

    //Debug
    void consistent();
    void findNull();

public :
    void addServer(ServerInfo &rServerInfo);

    void removeServer(int id);
    size_t numServers();
    size_t numServersInAgeList();

    ServerInfo getServer(int id);

    int getServerIDMatching(std::string ip, std::string port);
    Relay* createNewRelay(const std::string &rRelayBaseIdentiy, int port);
    void removeRelay(const std::string &rRelayIdentiy);

    idlist_t getServers(double maxTime, int minNumThreads=0, int maxNum=-1);
    void getOldestServer(int &rID, std::chrono::steady_clock::time_point &rTime);
    int getOldestServer();

    const int mMaxNumRunningRefreshServerStatusThreads = 30;
    std::atomic<int> mNumRunningRefreshServerStatusThreads{0};
    void refreshServerStatus(int serverId);
    void refreshServerBenchmark(int serverId);
};

#endif // SERVERHANDLER_H
