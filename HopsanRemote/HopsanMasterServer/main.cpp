#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include "zmq.hpp"
#include <map>
#include <list>
#include <chrono>

#include "Messages.h"
#include "MessageUtilities.h"
#include "ServerMessageUtilities.h"
#include "ServerStatusMessage.h"
#include "RemoteHopsanClient.h"

using namespace std;
using namespace std::chrono;

class ServerHandler;

class ServerInfo
{
    friend class ServerHandler;
public:
    std::string ip;
    std::string port;
    double benchmarkTime=1e100;
    steady_clock::time_point lastCheckTime;
    bool bussyProcessing=false;
    bool isReady;
private:
    size_t id;
};

class ServerHandler
{
private:
    typedef std::list<size_t> idlist_t;
    idlist_t mFreeIds;
    std::map<size_t, ServerInfo> mServerMap;
    std::mutex mLock;
public :
    void addServer(ServerInfo server)
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
        server.id = id;
        mServerMap.insert(std::pair<size_t, ServerInfo>(id,server));
        mLock.unlock();
    }

    void removeServer(size_t id)
    {
        mLock.lock();
        mServerMap.erase(id);
        mFreeIds.push_back(id);
        mLock.unlock();
    }

    ServerInfo getServer(size_t id)
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

    idlist_t getServersFasterThen(double maxTime, int maxNum=-1)
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

    size_t numServers()
    {
        return mServerMap.size();
    }

    idlist_t getServersToRefresh(double maxAge)
    {
        std::list<size_t> ids;
        for(auto &server : mServerMap)
        {
            duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - server.second.lastCheckTime);
            if (time_span.count() > maxAge)
            {
                ids.push_back(server.second.id);
            }
        }
        return ids;
    }
};



#define PRINTSERVER "HopsanMasterServer; "

ServerHandler gServerHandler;

// Prepare our context and socket
#ifdef _WIN32
zmq::context_t gContext(1, 63);
#else
zmq::context_t gContext(1);
#endif

void refreshServerStatus(size_t serverId)
{
    RemoteHopsanClient hopsanClient(gContext);
    ServerInfo server = gServerHandler.getServer(serverId);

    hopsanClient.connectToServer(server.ip, server.port);
    ServerStatusT status;
    bool rc = hopsanClient.requestStatus(status);

    if (rc)
    {
        // set stuff
        cout << "Server is responding!" << endl;
        server.lastCheckTime = steady_clock::now();

    }
    else
    {
        cout << "Server is not responding!" << endl;
        gServerHandler.removeServer(serverId);
        //! @todo what if network temporarily down
    }


}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << PRINTSERVER << "Error: you must specify what base port to use!" << endl;
        return 1;
    }
    string myPort = argv[1];

    cout << PRINTSERVER << "Listening on port: " << myPort  << endl;


    zmq::socket_t socket (gContext, ZMQ_REP);
    int linger_ms = 1000;
    socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));

    socket.bind( makeZMQAddress("*", myPort).c_str() );

    while (true)
    {
        // Wait for next request from client
        zmq::message_t message;
        if(receiveWithTimeout(socket, 30000, message))
        {
            size_t offset=0;
            size_t msg_id = getMessageId(message, offset);

            if (msg_id == S_Available)
            {


                SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset);
                cout << PRINTSERVER << "Server is available at IP: " << sm.ip << ":" << sm.port << endl;

                ServerInfo si;
                si.ip = sm.ip;
                si.port = sm.port;
                si.lastCheckTime = steady_clock::time_point(); // Epoch time

                sendServerAck(socket);


            }
            else if (msg_id == S_Closing)
            {
                SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset);
                cout << PRINTSERVER << "Server at IP: " << sm.ip << ":" << sm.port << " is closing!" << endl;

                // lookup server
                size_t id = 0;
                gServerHandler.removeServer(id);
                sendServerAck(socket);
            }
            else if (msg_id == C_ReqServerMachines)
            {
                //! @todo be smart
                CM_ReqServerMachines_t req = unpackMessage<CM_ReqServerMachines_t>(message,offset);
                cout << PRINTSERVER << "Got server machines request" << endl;
                auto ids = gServerHandler.getServersFasterThen(req.maxBenchmarkTime, req.numMachines);
                vector<string> ips, ports;
                for (auto id : ids)
                {
                    ServerInfo server = gServerHandler.getServer(id);
                    ips.push_back(server.ip);
                    ports.push_back(server.port);
                }

                MSM_ReqServerMachines_Reply_t reply;
                reply.ips = ips;
                reply.ports = ports;

                sendServerMessage<MSM_ReqServerMachines_Reply_t>(socket, S_ReqServerMachines_Reply, reply);

            }

        }

        double maxAge=60;
        std::list<size_t> refreshList = gServerHandler.getServersToRefresh(maxAge);
        if (!refreshList.empty())
        {
            //Spawn refresh threads
            for (auto &item : refreshList)
            {
                std::thread (refreshServerStatus, item ).detach();
            }


        }

        // check quit signal
    }

    return 0;
}
