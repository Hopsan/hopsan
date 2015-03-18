#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <iostream>
#include <thread>
#include <vector>
#include "zmq.hpp"


#include "Messages.h"
#include "MessageUtilities.h"
#include "ServerMessageUtilities.h"
#include "ServerStatusMessage.h"
#include "RemoteHopsanClient.h"

#include "ServerHandler.h"
#include "common.h"

using namespace std;
using namespace std::chrono;



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

    cout << PRINTSERVER << "Requesting status from server: " << serverId;
    hopsanClient.connectToServer(server.ip, server.port);
    ServerStatusT status;
    bool rc = hopsanClient.requestStatus(status);
    if (rc)
    {
        cout << " ... Server is responding!" << endl;
        server.lastCheckTime = steady_clock::now();

    }
    else
    {
        cout << " ... Server is NOT responding!" << endl;
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

                ServerInfo si;
                si.ip = sm.ip;
                si.port = sm.port;
                si.lastCheckTime = steady_clock::time_point(); // Epoch time

                if (gServerHandler.getServerMatching(sm.ip, sm.port) < 0)
                {
                    gServerHandler.addServer(si);
                    sendServerAck(socket);
                }
                else
                {
                    sendServerNAck(socket, "Address is already registered");
                }
            }
            else if (msg_id == S_Closing)
            {
                SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset);
                cout << PRINTSERVER << "Server at IP: " << sm.ip << ":" << sm.port << " is closing!" << endl;

                // lookup server
                //! @todo need to give servers a unique id to avoid others from beeing able to close them
                int id = gServerHandler.getServerMatching(sm.ip, sm.port);
                if (id >= 0)
                {
                    gServerHandler.removeServer(id);
                    sendServerAck(socket);
                }
                else
                {
                    sendServerNAck(socket, "You are no registered");
                }
            }
            else if (msg_id == C_ReqServerMachines)
            {
                //! @todo maybe refresh all before checking

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

        // Every time we get here we should refresh status from servers where status is old
        double maxAgeSeconds=60;
        std::list<size_t> refreshList = gServerHandler.getServersToRefresh(maxAgeSeconds);
        for (auto &item : refreshList)
        {
            // Spawn refresh threads (the thredas will dealcote themselves when done)
            std::thread (refreshServerStatus, item ).detach();
        }




        // check quit signal
    }

    return 0;
}
