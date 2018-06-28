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

#include <iostream>
#include <thread>
#include <vector>
#include <ctime>
#include <chrono>
#include <atomic>
#include <queue>
#include <cmath>

#include "zmq.hpp"
#include "hopsanremotecommon/Messages.h"
#include "hopsanremotecommon/MessageUtilities.h"
#include "hopsanremotecommon/StatusInfoStructs.h"
#include "RelayHandler.h"

#include "ServerHandler.h"
#include "common.h"

#include <tclap/CmdLine.h>

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
double gMaxAgeSeconds = 60*5;

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
    const double maxAgeSeconds=gMaxAgeSeconds;
    const int maxSleepSeconds=20;
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
        cout << PRINTSERVER << "Num RefreshThreads running: " << nRunning << " Num Servers: " << gServerHandler.numServers()
             << " Num AgeList: " << gServerHandler.numServersInAgeList() << endl;

        // If no servers are available, then sleep for a while
        if (gServerHandler.numServers() == 0)
        {
            std::chrono::milliseconds ms{maxSleepSeconds*1000};
            cout << "No servers, sleeping for: " << ms.count() << " milliseconds" << endl;
            std::this_thread::sleep_for(ms);
        }
        // Else if we have maxed out our refresh threads then sleep for a while
        else if (nRunning >= gServerHandler.mMaxNumRunningRefreshServerStatusThreads)
        {
            std::chrono::milliseconds ms{1000};
            cout << "Max num refresh threads running, sleeping for: " << ms.count() << " milliseconds" << endl;
            std::this_thread::sleep_for(ms);
        }
        // In this case we sleep a short time
        else if (gServerHandler.numServersInAgeList() == 0)
        {
            std::chrono::milliseconds ms{maxSleepSeconds/2*1000};
            cout << "No old servers, sleeping for: " << ms.count() << " milliseconds" << endl;
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
        //cout << "Processing id: " << id << endl;
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
            //cout << "Check id1==id2: " << id << " " << id2 << endl;
            if (id == id2)
            {
                // Spawn refresh threads (the threads will deallocate themselves when done)
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




std::string gSubnetMatch;
RelayHandler gRelayHandler;

std::string readIdentityEnvelope(zmq::socket_t &rSocket)
{
    zmq::message_t message, empty;
    rSocket.recv(&message);
    std::string identitydata(static_cast<char*>(message.data()), message.size());
    rSocket.recv(&empty);
    return identitydata;
}


void masterRelayThread(zmq::socket_t *pFrontend)
{
    cout << PRINTSERVER << nowDateTime() << " Starting relay thread!" << endl;

    //  Initialize poll set
    std::vector<zmq::pollitem_t> pollitems {{ (void*)*pFrontend, 0, ZMQ_POLLIN, 0 }};
    size_t ctrIn=0,ctrOut=0;

    while (true)
    {
        // Check break condition
        if (s_interrupted)
        {
            break;
        }

        zmq::message_t message;
        zmq::poll (pollitems, 10);

        if (pollitems[0].revents & ZMQ_POLLIN)
        {
            // Read identity
            std::string identity = readIdentityEnvelope(*pFrontend);

            // Read the actual message
            pFrontend->recv(&message);
            ctrIn++;

            cout << "Relaying message for identity: " << identity << endl;
//            size_t offset=0;
//            bool unpackok;
//            size_t id = getMessageId(message, offset, unpackok);
//            std::cout << "Message id:" << id << std::endl;

            // Lookup relay
            Relay *pRelay = gRelayHandler.getRelay(identity);
            if (pRelay)
            {
                pRelay->pushMessage(message);
            }
            else
            {
                sendIdentityEnvelope(*pFrontend, identity);
                sendMessage(*pFrontend, NotAck, "Invalid Relay Identity");
                cout << "Invalid Identity returniong NotAck" << endl;
                ctrOut++;
            }
        }

        // Send all waiting responses
        //! @todo need a response queue
        std::list<Relay*> relays = gRelayHandler.getRelays();
        for (Relay* pRelay : relays)
        {
            if (pRelay->haveResponse())
            {
                zmq::message_t response;
                pRelay->popResponse(response);

//                size_t offset=0;
//                bool unpackok;
//                size_t id = getMessageId(response, offset, unpackok);
//                std::cout << "response id:" << id << std::endl;

                std::string identity = pRelay->id();
                std::cout << "Responding to relay message identity: " << identity << std::endl;

                bool rc = sendIdentityEnvelope(*pFrontend, identity);
                if (rc)
                {
                    pFrontend->send(response);
                }
                ctrOut++;
                // Debug
                if (ctrIn != ctrOut)
                {
                    cout << "Warning: CtrIn:" << ctrIn << " CtrOut: " << ctrOut << endl;
                }
            }
        }
    }

    cout << PRINTSERVER << nowDateTime() << " Exiting relay thread!" << endl;
}

int main(int argc, char* argv[])
{
    TCLAP::CmdLine cmd("HopsanAddressServer", ' ', "0.1");

    // Define a value argument and add it to the command line.
    TCLAP::ValueArg<double> argRefreshTime("","refreshtime","The time between server status refresh",false,5,"minutes", cmd);
    TCLAP::ValueArg<std::string> argListenPort("p","port","The server listen port",true,"","Port number", cmd);
    TCLAP::ValueArg<std::string> argExternalIP("", "externalip", "The IP address to use for external connections if behind firewall", false, "", "ip address", cmd);
    TCLAP::ValueArg<std::string> argRelayPort("","relayport","The server relay port",false,"","Port number", cmd);
    TCLAP::ValueArg<std::string> argSubnetMatch("", "subnetmatch", "Subnet match filter", false, "", "", cmd);

    // Parse the argv array.
    cmd.parse( argc, argv );

    string myListenPort, myRelayPort, myExternalIP="Unknown";
    myListenPort = argListenPort.getValue();
    if (argRelayPort.isSet())
    {
        myRelayPort = argRelayPort.getValue();
    }
    else
    {
        myRelayPort = std::to_string(atoi(myListenPort.c_str())+1);
    }
    if (argExternalIP.isSet())
    {
        myExternalIP = argExternalIP.getValue();
    }
    gSubnetMatch = argSubnetMatch.getValue();

    cout << PRINTSERVER << nowDateTime() << " Listening on port: " << myListenPort  << endl;
    cout << PRINTSERVER << nowDateTime() << " My External IP: " << myExternalIP  << endl;
    if (!gSubnetMatch.empty())
    {
        cout << PRINTSERVER << nowDateTime() << " Relaying on port: " << myRelayPort  << endl;
        cout << PRINTSERVER << nowDateTime() << " Matching subnets: " << gSubnetMatch  << endl;
    }

    if (argRefreshTime.isSet())
    {
        gMaxAgeSeconds = argRefreshTime.getValue() * 60.0;
    }
    cout << PRINTSERVER << nowDateTime() << " Server refresh time: " << gMaxAgeSeconds << " seconds" << endl;

    try
    {
        zmq::socket_t socket (gContext, ZMQ_REP);
        zmq::socket_t frontend( gContext, ZMQ_ROUTER );

        int linger_ms = 1000;
        socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        frontend.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));

        socket.bind( makeZMQAddress("*", myListenPort).c_str() );
        frontend.bind( makeZMQAddress("*", myRelayPort).c_str() );

#ifdef _WIN32
        SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
#else
        s_catch_signals();
#endif

        // Start refresh thread
        std::thread refreshThread = std::thread( refreshServerThread );

        // Start relay thread
        std::thread relay_thread;
        if (!gSubnetMatch.empty())
        {
            relay_thread = std::thread( masterRelayThread, &frontend );
        }

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
                //cout << "msg_id: " << msg_id << endl;

                if (msg_id == ServerAvailable)
                {
                    bool parseOK;
                    InfomsgAvailable sm = unpackMessage<InfomsgAvailable>(message,offset,parseOK);

                    ServerInfo si;
                    si.address = sm.ip+":"+sm.port;
                    si.description = sm.description;
                    si.numTotalSlots = sm.numTotalSlots;
                    //! @todo services
                    si.lastCheckTime = steady_clock::time_point(); // Epoch time


                    if (gServerHandler.getServerIDMatching(sm.ip, sm.port) < 0)
                    {
                        gServerHandler.addServer(si);
                        sendShortMessage(socket, Ack);
                        // Get the oldest one, (should be the one we just added, since it need immediate update)
                        int id = gServerHandler.getOldestServer();
                        // Start a refresh thread for this one server, unless we have the maximum number of threads running
                        // In that case we let the ordinary refresh thread handle this server later
                        if (gServerHandler.mNumRunningRefreshServerStatusThreads < gServerHandler.mMaxNumRunningRefreshServerStatusThreads)
                        {
                            gServerHandler.refreshServerStatus(id);
                            std::chrono::milliseconds ms{50};
                            std::this_thread::sleep_for(ms);
                            //! @todo sleeping here is bad if many connect at the same time
                            gServerHandler.refreshServerBenchmark(id);
                        }
                        //! @todo if server is not responding here then additional adds will get the SAME server ID
                        // When we add a server, then request benchmark from it

                    }
                    else
                    {
                        sendMessage(socket, NotAck, "Address is already registered");
                    }
                }
                else if (msg_id == ServerClosing)
                {
                    bool parseOK;
                    InfomsgAvailable sm = unpackMessage<InfomsgAvailable>(message,offset,parseOK);
                    cout << PRINTSERVER << nowDateTime() << " Server at IP: " << sm.ip << ":" << sm.port << " is closing!" << endl;

                    // lookup server
                    //! @todo need to give servers a unique id to avoid others from being able to close them
                    int id = gServerHandler.getServerIDMatching(sm.ip, sm.port);
                    if (id >= 0)
                    {
                        gServerHandler.removeServer(id);
                        sendShortMessage(socket, Ack);
                    }
                    else
                    {
                        sendMessage(socket, NotAck, "You are not registered");
                    }
                }
                else if (msg_id == RequestServerMachines)
                {
                    //! @todo maybe refresh all before checking

                    //! @todo be smart
                    bool parseOK;
                    ReqmsgRequestServerMachines req = unpackMessage<ReqmsgRequestServerMachines>(message,offset,parseOK);
                    cout << PRINTSERVER << nowDateTime() << " Got server machines request" << endl;
                    auto ids = gServerHandler.getServers(req.maxBenchmarkTime, req.numMachines);
                    //! @todo what if a server is replaced or removed while we are processing this list

                    std::vector<ReplymsgReplyServerMachine> reply;
                    reply.reserve(ids.size());
                    for (auto id : ids)
                    {
                        ServerInfo server = gServerHandler.getServer(id);
                        if (server.isValid())
                        {
                            ReplymsgReplyServerMachine repl;
                            if (server.needsRelay())
                            {
                                repl.relayaddress = myExternalIP+":"+myRelayPort+":"+server.mRelayBaseIdentity;
                            }

                            repl.address = server.address;
                            repl.description = server.description;
                            repl.numslots = server.numTotalSlots;
                            repl.evalTime = server.benchmarkTime;

                            reply.push_back(repl);
                        }
                    }

                    cout << PRINTSERVER << nowDateTime() << " Responds with: " << reply.size() << " servers" << endl;
                    sendMessage(socket, ReplyServerMachines, reply);
                }
                else if (msg_id == RequestRelaySlot)
                {
                    bool parseOK;
                    ReqmsgRelaySlot req = unpackMessage<ReqmsgRelaySlot>(message,offset,parseOK);
                    cout << PRINTSERVER << nowDateTime() << " Got relay slot request, RelayBaseId: " << req.relaybaseid << " port: " << req.ctrlport << endl;
                    Relay* pRelay = nullptr;
                    if (!req.relaybaseid.empty())
                    {
                        // Port -1 means use server ctrl port
                        pRelay = gServerHandler.createNewRelay(req.relaybaseid, req.ctrlport);
                    }
                    if (pRelay)
                    {
                        pRelay->connectToEndpoint();
                        pRelay->startRelaying();

                        cout << PRINTSERVER << nowDateTime() << " Reserved RelayFullId: " << pRelay->id() << endl;
                        sendMessage(socket, ReplyRelaySlot, pRelay->id());
                    }
                    else
                    {
                        sendMessage(socket, NotAck, "Invalid Relay BaseId");
                    }
                }
                else if (msg_id == ReleaseRelaySlot)
                {
                    bool parseOK;
                    std::string relayslot_id = unpackMessage<std::string>(message,offset,parseOK);
                    cout << PRINTSERVER << nowDateTime() << " Releasing relay slot : " << relayslot_id << endl;

                    gRelayHandler.removeRelay(relayslot_id);
                    gServerHandler.removeRelay(relayslot_id);
                    sendShortMessage(socket, Ack);
                    gRelayHandler.purgeRemoved();
                }
                // Ignore the following messages silently
                else if (msg_id == ClientClosing)
                {
                    sendShortMessage(socket, Ack);
                }
                else if (!idParseOK)
                {
                    cout << PRINTSERVER << nowDateTime() << " Error: Could not parse message id" << std::endl;
                    sendMessage(socket, NotAck, "Could not parse message id");
                }
                else
                {
                    sendMessage(socket, NotAck, "Unhandled message");
                    cout << PRINTSERVER << nowDateTime() << "Warning: Unhandled message: " << msg_id << endl;
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

        if (!gSubnetMatch.empty())
        {
            cout << PRINTSERVER << nowDateTime() << " Waiting for relay thread..." << endl;
            relay_thread.join();
        }

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
