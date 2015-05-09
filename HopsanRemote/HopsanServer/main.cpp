//$Id$

#include <streambuf>
#include <sstream>
#include <cstdlib>

#include <iostream>
#include <vector>
#include "zmq.hpp"
#include <map>
#include <chrono>

#include "Messages.h"
#include "MessageUtilities.h"
#include "ServerMessageUtilities.h"

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#endif

using namespace std;
using namespace std::chrono;

extern char **environ;

class WorkerInfo
{
public:
    int numThreads;
#ifdef _WIN32
    PROCESS_INFORMATION pi;
#else
    pid_t pid;
#endif

};

class ServerConfig
{
public:
    int mControlPort = 23300;
    string mControlPortStr = "23300";
    int mMaxNumSlots = 4;
};

ServerConfig gServerConfig;
size_t nTakenSlots=0;
#ifdef _WIN32
zmq::context_t gContext(1, 63);
#else
zmq::context_t gContext(1);
#endif

#define PRINTSERVER "HopsanServer@"+gServerConfig.mControlPortStr+"; "
std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return string(&buff[0]);
}

bool readAckNackServerMessage(zmq::socket_t &rSocket, long timeout, string &rNackReason)
{
    zmq::message_t response;
    if(receiveWithTimeout(rSocket, timeout, response))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        //cout << "id: " << id << endl;
        if (id == S_Ack)
        {
            return true;
        }
        else if (id == S_NAck)
        {
            rNackReason = unpackMessage<std::string>(response, offset, parseOK);
        }
        else if (!parseOK)
        {
            rNackReason = "Exception in msgpack:unpack!";
        }
        else
        {
            rNackReason = "Got neither Ack nor Nack";
        }
    }
    else
    {
        rNackReason = "Got either timeout or exception in zmq::recv()";
    }
    return false;
}

void reportToAddressServer(std::string addressIP, std::string addressPort, std::string myIP, std::string myPort, bool isOnline)
{
    try
    {
        zmq::socket_t addressServerSocket (gContext, ZMQ_REQ);
        int linger_ms = 1000;
        addressServerSocket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        addressServerSocket.connect(makeZMQAddress(addressIP, addressPort).c_str());

        //! @todo check if ok
        SM_Available_t message;
        message.ip = myIP;
        message.port = myPort;

        if (isOnline)
        {
            sendServerMessage(addressServerSocket, S_Available, message);
            std::string nackreason;
            bool ack = readAckNackServerMessage(addressServerSocket, 5000, nackreason);
            if (ack)
            {
                cout << PRINTSERVER << nowDateTime() << " Successfully registered in address server" << endl;
            }
            else
            {

                cout << PRINTSERVER << nowDateTime() << " Error: Could not register in master server: " << nackreason << endl;
            }
        }
        else
        {
            sendServerMessage(addressServerSocket, S_Closing, message);
            std::string nackreason;
            bool ack = readAckNackServerMessage(addressServerSocket, 5000, nackreason);
            if (ack)
            {
                cout << PRINTSERVER << nowDateTime() << " Successfully unregistered in address server" << endl;
            }
            else
            {
                cout << PRINTSERVER << nowDateTime() << " Error: Could not unregister in master server: " << nackreason << endl;
            }
        }
        addressServerSocket.disconnect(makeZMQAddress(addressIP, addressPort).c_str());
    }
    catch(zmq::error_t e)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: Preparing addressServerSocket: " << e.what() << endl;
    }
}

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

map<int, WorkerInfo> workerMap;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: you must specify what base port to use!" << endl;
        return 1;
    }
    string myPort = argv[1];
    gServerConfig.mControlPortStr = myPort;
    gServerConfig.mControlPort = atoi(myPort.c_str());

    std::string masterserverip, masterserverport;
    steady_clock::time_point lastStatusRequestTime;
    if (argc >= 4)
    {
        masterserverip = argv[2];
        masterserverport = argv[3];
    }

    string myExternalIP = "127.0.0.1";
    if (argc >= 5)
    {
        myExternalIP = argv[4];
    }

    cout << PRINTSERVER << nowDateTime() << " Listening on port: " << myPort  << endl;

    // Prepare our context and socket
    try
    {
        zmq::socket_t socket (gContext, ZMQ_REP);
        int linger_ms = 1000;
        socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        socket.bind( makeZMQAddress("*", myPort).c_str() );

        if (!masterserverip.empty())
        {
            //! @todo somehow automatically figure out my ip
            reportToAddressServer(masterserverip, masterserverport, myExternalIP, myPort, true);
        }

#ifdef _WIN32
        SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
#else
        s_catch_signals();
#endif
        while (true)
        {
            // Wait for next request from client
            zmq::message_t request;
            if(receiveWithTimeout(socket, 30000, request))
            {
                size_t offset=0;
                bool idParseOK;
                size_t msg_id = getMessageId(request, offset, idParseOK);
                //cout << PRINTSERVER << nowDateTime() << " Received message with length: " << request.size() << " msg_id: " << msg_id << endl;

                //        int fd = request.get(ZMQ_SRCFD);
                //        struct sockaddr addr;
                //        unsigned int len;
                //        getpeername(fd, &addr, &len);
                //        cout << "Addr len: " << len << " data: ";
                //        for (int i=0; i<len; ++i)
                //        {
                //            cout << addr.sa_data[i];
                //        }
                //        cout << endl;

                //        string soxtype = request.gets("Socket-Type");
                //        string identity = request.gets("Identity");
                //        cout << "Socket-type: " << soxtype << " Identity: " << identity << endl;

                if (msg_id == C_ReqSlot)
                {
                    bool parseOK;
                    CM_ReqSlot_t msg = unpackMessage<CM_ReqSlot_t>(request, offset, parseOK);
                    int requestNumThreads = msg.numThreads;

                    cout << PRINTSERVER << nowDateTime() << " Client is requesting: " << requestNumThreads << " slots... " << endl;
                    if (nTakenSlots+requestNumThreads <= gServerConfig.mMaxNumSlots)
                    {
                        size_t workerPort = gServerConfig.mControlPort+nTakenSlots+1;

                        // Generate unique worker Id
                        int uid = rand();
                        while (workerMap.count(uid) != 0)
                        {
                            uid = rand();
                        }

#ifdef _WIN32
                        PROCESS_INFORMATION processInformation;
                        STARTUPINFO startupInfo;
                        memset(&processInformation, 0, sizeof(processInformation));
                        memset(&startupInfo, 0, sizeof(startupInfo));
                        startupInfo.cb = sizeof(startupInfo);

                        string scport = to_string(gServerConfig.mControlPort);
                        string swport = to_string(workerPort);
                        string nthreads = to_string(requestNumThreads);
                        string uidstr = to_string(uid);

                        std::string appName("HopsanServerWorker.exe");
                        std::string cmdLine("HopsanServerWorker "+uidstr+" "+scport+" "+swport+" "+nthreads);
                        TCHAR tempCmdLine[cmdLine.size()*2];
                        strcpy_s(tempCmdLine, cmdLine.size()*2, cmdLine.c_str());

                        BOOL result = CreateProcess(appName.c_str(), tempCmdLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInformation);
                        if (result == 0)
                        {
                            std::cout << PRINTSERVER << "Error: Failed to launch worker process!"<<endl;
                            sendServerNAck(socket, "Failed to launch worker process!");
                        }
                        else
                        {
                            std::cout << PRINTSERVER << "Launched Worker Process, pid: "<< processInformation.dwProcessId << " port: " << port << " uid: " << uid << endl;
                            workerMap.insert({uid, {requestNumThreads, processInformation}});

                            SM_ReqSlot_Reply_t msg = {port};
                            sendServerMessage<SM_ReqSlot_Reply_t>(socket, S_ReqSlot_Reply, msg);
                            nTakenSlots++;
                        }

#else
                        char sport_buff[64], wport_buff[64], thread_buff[64], uid_buff[64];
                        // Write port as char in buffer
                        sprintf(sport_buff, "%d", gServerConfig.mControlPort);
                        sprintf(wport_buff, "%d", int(workerPort));
                        // Write num threads as char in buffer
                        sprintf(thread_buff, "%d", requestNumThreads);
                        // Write id as char in buffer
                        sprintf(uid_buff, "%d", uid);

                        char *argv[] = {"HopsanServerWorker", uid_buff, sport_buff, wport_buff, thread_buff, nullptr};

                        pid_t pid;
                        int status = posix_spawn(&pid,"./HopsanServerWorker",nullptr,nullptr,argv,environ);
                        if(status == 0)
                        {
                            std::cout << PRINTSERVER << nowDateTime() << " Launched Worker Process, pid: "<< pid << " port: " << workerPort << " uid: " << uid << endl;
                            workerMap.insert({uid,{requestNumThreads,pid}});

                            SM_ReqSlot_Reply_t msg = {workerPort};
                            sendServerMessage<SM_ReqSlot_Reply_t>(socket, S_ReqSlot_Reply, msg);
                            nTakenSlots+=requestNumThreads;
                        }
                        else
                        {
                            std::cout << PRINTSERVER << nowDateTime() << " Error: Failed to launch worker process!"<<endl;
                            sendServerNAck(socket, "Failed to launch worker process!");
                        }
#endif
                    }
                    else if (nTakenSlots == gServerConfig.mMaxNumSlots)
                    {
                        sendServerNAck(socket, "All slots taken");
                        cout << PRINTSERVER << nowDateTime() << " Denied! All slots taken." << endl;
                    }
                    else
                    {
                        sendServerNAck(socket, "To few free slots");
                        cout << PRINTSERVER << nowDateTime() << " Denied! To few free slots." << endl;
                    }
                }
                else if (msg_id == SW_Finished)
                {
                    bool parseOK;
                    string id_string = unpackMessage<std::string>(request,offset,parseOK);
                    if (parseOK)
                    {
                        int id = atoi(id_string.c_str());
                        cout << PRINTSERVER << nowDateTime() << " Worker " << id_string << " Finished!" << endl;

                        auto it = workerMap.find(id);
                        if (it != workerMap.end())
                        {
                            sendServerAck(socket);

                            // Wait for process to stop (to avoid zombies)
#ifdef _WIN32
                            PROCESS_INFORMATION pi = it->second.pi;
                            WaitForSingleObject( pi.hProcess, INFINITE );
                            CloseHandle( pi.hProcess );
                            CloseHandle( pi.hThread );
#else
                            pid_t pid = it->second.pid;
                            int stat_loc;
                            pid_t status = waitpid(pid, &stat_loc, WUNTRACED);
#endif
                            //! @todo check returncodes maybe
                            workerMap.erase(it);
                            nTakenSlots--;
                        }
                        else
                        {
                            sendServerNAck(socket, "Wrong worker id specified");
                        }
                    }
                    else
                    {
                        cout << PRINTSERVER << nowDateTime() << " Error: Could not server id string" << endl;
                    }
                }
                else if (msg_id == C_ReqStatus)
                {
                    cout << PRINTSERVER << nowDateTime() << " Client is requesting status" << endl;
                    SM_ServerStatus_t status;
                    status.numTotalSlots = gServerConfig.mMaxNumSlots;
                    status.numFreeSlots = gServerConfig.mMaxNumSlots-nTakenSlots;
                    status.isReady = (status.numFreeSlots > 0);

                    sendServerMessage<SM_ServerStatus_t>(socket, S_ReqStatus_Reply, status);
                    lastStatusRequestTime = chrono::steady_clock::now();
                }
                else if (!idParseOK)
                {
                    cout << PRINTSERVER << nowDateTime() << " Error: Could not parse message id" << endl;
                }
                else
                {
                    stringstream ss;
                    ss << PRINTSERVER << nowDateTime() << " Error: Unknown message id " << msg_id << endl;
                    cout << ss.str() << endl;
                    sendServerNAck(socket, ss.str());
                }
            }
            else
            {
                // Handle timeout / exception
            }

            duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - lastStatusRequestTime);
            if (time_span.count() > 120)
            {
                cout << PRINTSERVER << nowDateTime() << " Too long since status check: " << time_span.count() << endl;

                // If noone has requested status for this long (and we have a master server) we assume that the master server
                // has gone down, lets reconnect to it to make sure it knows that we still exist
                //! @todo maybe this should be handled in the server by saving known servers to file instead
                if (!masterserverip.empty())
                {
                    reportToAddressServer(masterserverip, masterserverport, myExternalIP, myPort, true);
                }
            }

            // Do some 'work'
#ifndef _WIN32
            //sleep(1);
#else
            //Sleep (1);
#endif

            if (s_interrupted)
            {
                cout << PRINTSERVER << nowDateTime() << " Interrupt signal received, killing server" << std::endl;
                break;
            }
        }

        // Tell master server we are closing
        if (!masterserverip.empty())
        {
            //! @todo somehow automatically figure out my ip
            reportToAddressServer(masterserverip, masterserverport, myExternalIP, myPort, false);
        }

    }
    catch(zmq::error_t e)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: Preparing our context and socket: " << e.what() << endl;
    }

    cout << PRINTSERVER << nowDateTime() << " Closed!" << endl;
}

