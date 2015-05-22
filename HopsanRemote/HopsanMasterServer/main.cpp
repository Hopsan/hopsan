#include <iostream>
#include <thread>
#include <vector>
#include <ctime>
#include <chrono>
#include <atomic>

#include "zmq.hpp"
#include "Messages.h"
#include "MessageUtilities.h"
#include "ServerMessageUtilities.h"
#include "ServerStatusMessage.h"


#include "ServerHandler.h"
#include "common.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

using namespace std;
using namespace std::chrono;

std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return std::string(&buff[0]);
}

// Create our zmq::context
#ifdef _WIN32
zmq::context_t gContext(1, 63);
#else
zmq::context_t gContext(1);
#endif

ServerHandler gServerHandler;

static int s_interrupted = 0;
#ifdef _WIN32
BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
{
    // what to do here?
    s_interrupted = 1;
    return TRUE;
}
#else
static void s_signal_handler(int signal_value)
{
    s_interrupted = 1;
}

static void s_catch_signals(void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}
#endif


//! @todo this thread should be a member of the server handler
void refreshServerThread()
{
    const double maxAgeSeconds=120;
    cout << PRINTSERVER << nowDateTime() << " Starting server refresh thread!" << endl;

    while (true)
    {
        // Check break condition
        if (s_interrupted)
        {
            break;
        }

        // Extract oldest servers
        int id=-1, id2=-1;
        steady_clock::time_point tp, tp2;

        int nRunning = gServerHandler.mNumRunningRefreshServerStatusThreads;
        cout << "Num RefreshThreads running: " << nRunning << endl;
        cout << "Num Servers: " << gServerHandler.numServers() << endl;

        // If no servers are available, then sleep for a while
        if (gServerHandler.numServers() == 0)
        {
            std::chrono::milliseconds ms{int(floor(maxAgeSeconds*1000))};
            cout << "No servers sleeping for: " << ms.count() << " milliseconds" << endl;
            std::this_thread::sleep_for(ms);
        }
        // Else if we have maxed out our refresh threads then sleep for a while
        else if (nRunning >= gServerHandler.mMaxNumRunningRefreshServerStatusThreads)
        {
            std::chrono::milliseconds ms{1000};
            cout << "Max num refresh threads running, sleeping for: " << ms.count() << " milliseconds" << endl;
            std::this_thread::sleep_for(ms);
        }
        // Else we fetch the oldest server
        else
        {
            gServerHandler.getOldestServer(id,tp);
        }

        // Check break condition
        if (s_interrupted)
        {
            break;
        }

        // If we got a server, then process it, else skip
        if (id >= 0)
        {
            // If the oldest server is not old enough for refresh then lets wait until it is
            duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - tp);
            if (time_span.count() < maxAgeSeconds)
            {
                // Sleep until its time
                std::chrono::milliseconds ms{int(floor((maxAgeSeconds-time_span.count())*1000))};
                cout << "No server needs update, Sleeping for: " << ms.count() << " milliseconds" << endl;
                std::this_thread::sleep_for(ms);
            }

            // Now the oldest server should be old enough, but lets request oldest again just to be sure it has not been removed
            gServerHandler.getOldestServer(id2,tp2);
            // If oldest is still the same then spawn a refresh thread for it, else we do nothing
            if (id == id2)
            {
                // Spawn refresh threads (the threds will dealocate themselves when done)
                //std::thread (refreshServerStatus, server_id ).detach();
                //std::thread (&ServerHandler::refreshServerStatus, &gServerHandler, id).detach();
                gServerHandler.refreshServerStatus(id);

                // Debug sleep to let cout print nicely, remove later
                std::chrono::milliseconds ms{50};
                std::this_thread::sleep_for(ms);

                if (gServerHandler.getServer(id).benchmarkTime > 1e99)
                {
                    gServerHandler.refreshServerBenchmark(id);
                }

                // Debug sleep to let cout print nicely, remove later
                std::this_thread::sleep_for(ms);
            }
        }
    }
    cout << PRINTSERVER << nowDateTime() << " Exiting server refresh thread!" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: you must specify what base port to use!" << endl;
        return 1;
    }
    string myPort = argv[1];

    cout << PRINTSERVER << nowDateTime() << " Listening on port: " << myPort  << endl;

    try
    {
        zmq::socket_t socket (gContext, ZMQ_REP);
        int linger_ms = 1000;
        socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        socket.bind( makeZMQAddress("*", myPort).c_str() );

#ifdef _WIN32
        SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
#else
        s_catch_signals();
#endif

        // Start refresh thread
        std::thread refreshThread = std::thread( refreshServerThread );

        // Start main thread
        while (true)
        {
            // Wait for next request from client
            zmq::message_t message;
            if(receiveWithTimeout(socket, 30000, message))
            {
                size_t offset=0;
                bool idParseOK;
                size_t msg_id = getMessageId(message, offset, idParseOK);

                if (msg_id == S_Available)
                {
                    bool parseOK;
                    SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset,parseOK);

                    ServerInfo si;
                    si.ip = sm.ip;
                    si.port = sm.port;
                    si.description = sm.description;
                    si.lastCheckTime = steady_clock::time_point(); // Epoch time

                    if (gServerHandler.getServerIDMatching(sm.ip, sm.port) < 0)
                    {
                        gServerHandler.addServer(si);
                        sendServerAck(socket);
                        // Get the oldest one, (should be the one we just added, since it need emmediate update)
                        int id = gServerHandler.getOldestServer();
                        // Start a refresh thread for this one server, unless we have the maximum number of threads running
                        // In that case we let the ordinary refresh thread handle this server later
                        if (gServerHandler.mNumRunningRefreshServerStatusThreads < gServerHandler.mMaxNumRunningRefreshServerStatusThreads)
                        {
                            gServerHandler.refreshServerStatus(id);
                            std::chrono::milliseconds ms{50};
                            std::this_thread::sleep_for(ms);
                            //! @todo sleeping here is bad if manny connect at the same time
                            gServerHandler.refreshServerBenchmark(id);
                        }
                        //! @todo if server is not responnding here tehn additional adds will get the SAME server ID
                        // When we add a server, then request benchmark from it

                    }
                    else
                    {
                        sendServerNAck(socket, "Address is already registered");
                    }
                }
                else if (msg_id == S_Closing)
                {
                    bool parseOK;
                    SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset,parseOK);
                    cout << PRINTSERVER << nowDateTime() << " Server at IP: " << sm.ip << ":" << sm.port << " is closing!" << endl;

                    // lookup server
                    //! @todo need to give servers a unique id to avoid others from beeing able to close them
                    int id = gServerHandler.getServerIDMatching(sm.ip, sm.port);
                    if (id >= 0)
                    {
                        gServerHandler.removeServer(id);
                        sendServerAck(socket);
                    }
                    else
                    {
                        sendServerNAck(socket, "You are not registered");
                    }
                }
                else if (msg_id == C_ReqServerMachines)
                {
                    //! @todo maybe refresh all before checking

                    //! @todo be smart
                    bool parseOK;
                    CM_ReqServerMachines_t req = unpackMessage<CM_ReqServerMachines_t>(message,offset,parseOK);
                    cout << PRINTSERVER << nowDateTime() << " Got server machines request" << endl;
                    auto ids = gServerHandler.getServers(req.maxBenchmarkTime, req.numMachines);
                    //! @todo what if a server is replaced or removed while we are processing this list

                    MSM_ReqServerMachines_Reply_t reply;
                    reply.ips.reserve(ids.size());
                    reply.ports.reserve(ids.size());
                    reply.descriptions.reserve(ids.size());
                    reply.numslots.reserve(ids.size());
                    reply.speeds.reserve(ids.size());
                    for (auto id : ids)
                    {
                        ServerInfo server = gServerHandler.getServer(id);
                        if (server.isValid())
                        {
                            reply.ips.push_back(server.ip);
                            reply.ports.push_back(server.port);
                            reply.descriptions.push_back(server.description);
                            reply.numslots.push_back(server.numTotalSlots);
                            reply.speeds.push_back(server.benchmarkTime);
                        }
                    }

                    cout << PRINTSERVER << nowDateTime() << " Responds with: " << reply.ips.size() << " servers" << endl;
                    sendServerMessage<MSM_ReqServerMachines_Reply_t>(socket, S_ReqServerMachines_Reply, reply);
                }
                else if (!idParseOK)
                {
                    cout << PRINTSERVER << nowDateTime() << " Error: Could not parse message id" << std::endl;
                }

            }

            // Check quit signal
            if (s_interrupted)
            {
                cout << PRINTSERVER << nowDateTime() << " Interrupt signal received, killing server" << std::endl;
                break;
            }
        }

        s_interrupted = 1;
        cout << PRINTSERVER << nowDateTime() << " Waiting for server refresh thread..." << endl;
        refreshThread.join();
    }
    catch (zmq::error_t  e)
    {
        cout << "Error: Could not create sockets: " << e.what() << endl;
    }

    cout << PRINTSERVER << nowDateTime() << " Closed!" << endl;
    return 0;
}
