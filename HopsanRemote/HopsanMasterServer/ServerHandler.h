#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <list>
#include <atomic>
#include <vector>

class ServerHandler;

class ServerInfo
{
    friend class ServerHandler;
private:
    int mId=-1;

public:
    std::string ip;
    std::string port;
    std::string description;
    int numTotalSlots = 0;
    double benchmarkTime=1e100;
    std::vector<double> benchmarkTimes;
    std::chrono::steady_clock::time_point lastCheckTime;
    bool bussyProcessing=false;
    bool isReady=false;

    inline int id() const
    {
        return mId;
    }

    inline bool isValid() const
    {
        return (mId >= 0);
    }
};

#define BENCHMARKMODEL "../Models/Example Models/Load Sensing System.hmf"

class ServerHandler
{
private:
    typedef std::list<int> idlist_t;
    idlist_t mFreeIds;
    idlist_t mServerAgeList;
    idlist_t mServerRefreshList;
    std::map<int, ServerInfo> mServerMap;
    std::mutex mMutex;

    ServerInfo getServerNoLock(int id);
    void updateServerInfoNoLock(const ServerInfo &rServerInfo);

    void refreshServerStatusThread(int serverId);
    void refreshServerBenchmarkThread(int serverId);

    //Debug
    void consistent();
    void findNull();

public :
    void addServer(ServerInfo &rServerInfo);

    void removeServer(int id);
    size_t numServers();

    ServerInfo getServer(int id);

    int getServerIDMatching(std::string ip, std::string port);
    idlist_t getServers(double maxTime, int minNumThreads=0, int maxNum=-1);
    void getOldestServer(int &rID, std::chrono::steady_clock::time_point &rTime);
    int getOldestServer();

    const int mMaxNumRunningRefreshServerStatusThreads = 30;
    std::atomic<int> mNumRunningRefreshServerStatusThreads{0};
    void refreshServerStatus(int serverId);
    void refreshServerBenchmark(int serverId);
};

#endif // SERVERHANDLER_H
