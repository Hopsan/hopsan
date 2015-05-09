#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <list>
#include <atomic>

class ServerHandler;

class ServerInfo
{
    friend class ServerHandler;
private:
    int mId=-1;

public:
    std::string ip;
    std::string port;
    double benchmarkTime=1e100;
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

class ServerHandler
{
private:
    typedef std::list<int> idlist_t;
    idlist_t mFreeIds;
    idlist_t mServerAgeList;
    //idlist_t mServerRefreshList;
    std::map<int, ServerInfo> mServerMap;
    std::mutex mMutex;

    ServerInfo getServerNoLock(int id);
    void updateServerInfoNoLock(const ServerInfo &rServerInfo);

    void refreshServerStatusThread(size_t serverId);

    //Debug
    void consistent();
    void findNull();

public :
    void addServer(ServerInfo &rServerInfo);

    void removeServer(int id);
    size_t numServers();

    ServerInfo getServer(int id);

    int getServerIDMatching(std::string ip, std::string port);
    //std::chrono::steady_clock::time_point getServerAge(int id);
    idlist_t getServersFasterThen(double maxTime, int maxNum=-1);
    //idlist_t getServersToRefresh(double maxAge, int maxNumServers=-1);
    //idlist_t getOldestServers(size_t maxNum);
    void getOldestServer(int &rID, std::chrono::steady_clock::time_point &rTime);
    int getOldestServer();

    const int mMaxNumRunningRefreshServerStatusThreads = 20;
    std::atomic<int> mNumRunningRefreshServerStatusThreads{0};
    void refreshServerStatus(size_t serverId);
};

#endif // SERVERHANDLER_H
