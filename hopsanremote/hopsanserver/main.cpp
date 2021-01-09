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

//$Id$

#include <streambuf>
#include <sstream>
#include <cstdlib>

#include <iostream>
#include <vector>
#include <map>
#include <chrono>

#include "hopsanremotecommon/Messages.h"
#include "hopsanremotecommon/MessageUtilities.h"

#include <tclap/CmdLine.h>
#include "zmq.hpp"

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
#ifdef _WIN32
    WorkerInfo(int numSlots, size_t port, string userid, PROCESS_INFORMATION pid)
#else
    WorkerInfo(int numSlots, size_t port, string userid, pid_t pid)
#endif
    {
        mNumSlots = numSlots;
        mWorkerPort = port;
        mUserid = userid;
        mPid = pid;
        mLastAliveReport = steady_clock::now();
    }

    int mNumSlots=0;
    size_t mWorkerPort;
    string mUserid;
    steady_clock::time_point mLastAliveReport;
#ifdef _WIN32
    PROCESS_INFORMATION mPid;
#else
    pid_t mPid;
#endif

};

class ServerConfig
{
public:
    int mControlPort = 23300;
    string mControlPortStr = "23300";
    int mMaxNumSlots = 4;
    string mDescription;
    string mExternalIP;
    string mAddressServerIPandPort;
    double mAddressReportAge = 60*10;
};

ServerConfig gServerConfig;
int gNumTakenSlots=0;
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


bool receiveAckNackMessage(zmq::socket_t &rSocket, long timeout, string &rNackReason)
{
    zmq::message_t response;
    if(receiveWithTimeout(rSocket, timeout, response))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        //cout << "id: " << id << endl;
        if (id == Ack)
        {
            return true;
        }
        else if (id == NotAck)
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

void reportToAddressServer(const ServerConfig &rServerConfig, bool isOnline)
{
    try
    {
        zmq::socket_t addressServerSocket (gContext, ZMQ_REQ);
        int linger_ms = 1000;
        addressServerSocket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        addressServerSocket.connect(makeZMQAddress(rServerConfig.mAddressServerIPandPort).c_str());

        //! @todo check if OK
        InfomsgAvailable message;
        message.ip = rServerConfig.mExternalIP;
        message.port = rServerConfig.mControlPortStr;
        message.description = rServerConfig.mDescription;
        message.numTotalSlots = gServerConfig.mMaxNumSlots;
        message.identity = 0; // Identity should always be 0 when a server starts, relay servers may set other values

        if (isOnline)
        {
            sendMessage(addressServerSocket, ServerAvailable, message);
            std::string nackreason;
            bool ack = receiveAckNackMessage(addressServerSocket, 5000, nackreason);
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
            sendMessage(addressServerSocket, ServerClosing, message);
            std::string nackreason;
            bool ack = receiveAckNackMessage(addressServerSocket, 5000, nackreason);
            if (ack)
            {
                cout << PRINTSERVER << nowDateTime() << " Successfully unregistered in address server" << endl;
            }
            else
            {
                cout << PRINTSERVER << nowDateTime() << " Error: Could not unregister in master server: " << nackreason << endl;
            }
        }
        addressServerSocket.disconnect(makeZMQAddress(gServerConfig.mAddressServerIPandPort).c_str());
    }
    catch(zmq::error_t e)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: Preparing addressServerSocket: " << e.what() << endl;
    }
}


bool checkIfWorkerIsAlive(WorkerInfo &rWI)
{
    bool isAlive = false;
    try
    {
        zmq::socket_t workerSocket (gContext, ZMQ_REQ);
        int linger_ms = 1000;
        workerSocket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        workerSocket.connect(makeZMQAddress("127.0.0.1", rWI.mWorkerPort).c_str());

        sendShortMessage(workerSocket, RequestWorkerStatus);
        zmq::message_t response;
        if (receiveWithTimeout(workerSocket, 1000, response))
        {
            size_t offset=0; bool parseOK;
            size_t id = getMessageId(response, offset, parseOK);
            if (id == ReplyWorkerStatus)
            {
                ReplymsgReplyWorkerStatus status = unpackMessage<ReplymsgReplyWorkerStatus>(response, offset, parseOK);
                if (parseOK)
                {
                    isAlive = true;
                }
            }
        }
        workerSocket.disconnect(makeZMQAddress("127.0.0.1", rWI.mWorkerPort).c_str());
    }
    catch(zmq::error_t e)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: Contacting Worker: " << e.what() << endl;
    }
    rWI.mLastAliveReport = steady_clock::now();
    return isAlive;
}

void waitForWorkerProcess(WorkerInfo &rWI)
{
#ifdef _WIN32
    PROCESS_INFORMATION pi = rWI.mPid;
    WaitForSingleObject( pi.hProcess, INFINITE );
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else
    pid_t pid = rWI.mPid;
    int stat_loc;
    pid_t status = waitpid(pid, &stat_loc, WUNTRACED);
#endif
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
    TCLAP::CmdLine cmd("HopsanServer", ' ', "0.1");

    // Define a value argument and add it to the command line.
    TCLAP::ValueArg<double> argAddressReportAge("","addressreportage","Report to address server if no status request received for this time", false, 10, "minutes", cmd);
    TCLAP::ValueArg<int> argBasePort("p","port","The server listen base port",true,50100,"Port number (int)", cmd);
    TCLAP::ValueArg<int> argNumSlots("n","numslots","The number of slots (threads) to use",true,2,"int", cmd);
    TCLAP::ValueArg<std::string> argExternalIP("", "externalip", "The external IP address to report (for use behind NAT)", false, "127.0.0.1", "ip address", cmd);

    TCLAP::ValueArg<std::string> argDescription("", "description", "Label for this server", false, "", "", cmd);
    TCLAP::ValueArg<std::string> argAddressServerIP("", "addresserver", "IP:port to address server", false, "", "", cmd);

    // Parse the argv array.
    cmd.parse( argc, argv );

    gServerConfig.mMaxNumSlots = argNumSlots.getValue();
    gServerConfig.mControlPortStr = to_string(argBasePort.getValue());
    gServerConfig.mControlPort = argBasePort.getValue();
    gServerConfig.mDescription = argDescription.getValue();
    gServerConfig.mExternalIP = argExternalIP.getValue();
    gServerConfig.mAddressServerIPandPort = argAddressServerIP.getValue();
    gServerConfig.mAddressReportAge = argAddressReportAge.getValue()*60;

    steady_clock::time_point lastStatusRequestTime;

    cout << PRINTSERVER << nowDateTime() << " Starting with: " << gServerConfig.mMaxNumSlots << " slots, Listening on port: " << gServerConfig.mControlPort  << endl;

    // Prepare our context and socket
    try
    {
        zmq::socket_t socket (gContext, ZMQ_REP);
        int linger_ms = 1000;
        socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        socket.bind( makeZMQAddress("*", argBasePort.getValue()).c_str() );

        if (argAddressServerIP.isSet())
        {
            //! @todo somehow automatically figure out my IP
            reportToAddressServer(gServerConfig, true);
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

                if (msg_id == RequestServerSlots)
                {
                    bool parseOK;
                    ReqmsgReqServerSlots msg = unpackMessage<ReqmsgReqServerSlots>(request, offset, parseOK);
                    int requestNumThreads = msg.numThreads;
                    string requestuserid = msg.userid;
                    if (requestuserid.empty())
                    {
                        requestuserid = "anonymous";
                    }

                    cout << PRINTSERVER << nowDateTime() << " Client (" << requestuserid << ") is requesting: " << requestNumThreads << " slots... " << endl;
                    if (gNumTakenSlots+requestNumThreads <= gServerConfig.mMaxNumSlots)
                    {
                        size_t workerPort = gServerConfig.mControlPort+gNumTakenSlots+1;

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

                        std::string appName("hopsanserverworker.exe");
                        std::string cmdLine("hopsanserverworker "+uidstr+" "+scport+" "+swport+" "+nthreads);
                        TCHAR* pTCharCmdLineBuff = new TCHAR[cmdLine.size()+1];
                        strcpy_s(pTCharCmdLineBuff, cmdLine.size()+1, cmdLine.c_str());

                        BOOL result = CreateProcess(appName.c_str(), pTCharCmdLineBuff, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInformation);
                        if (result == 0)
                        {
                            std::cout << PRINTSERVER << "Error: Failed to launch worker process!"<<endl;
                            sendMessage(socket, NotAck, "Failed to launch worker process!");
                        }
                        else
                        {
                            std::cout << PRINTSERVER << "Launched Worker Process, pid: "<< processInformation.dwProcessId << " port: " << workerPort << " uid: " << uid << " nThreads: " << requestNumThreads  << endl;
                            workerMap.insert({uid, WorkerInfo(requestNumThreads, workerPort, requestuserid, processInformation)});

                            ReplymsgReplyServerSlots msg = {workerPort};
                            sendMessage<ReplymsgReplyServerSlots>(socket, ReplyServerSlots, msg);
                            gNumTakenSlots++;
                        }
                        delete pTCharCmdLineBuff;

#else
                        char name_buff[64], sport_buff[64], wport_buff[64], thread_buff[64], uid_buff[64];
                        // Write name
                        sprintf(name_buff, "%s", "hopsanserverworker");
                        // Write port as char in buffer
                        sprintf(sport_buff, "%d", gServerConfig.mControlPort);
                        sprintf(wport_buff, "%d", int(workerPort));
                        // Write num threads as char in buffer
                        sprintf(thread_buff, "%d", requestNumThreads);
                        // Write id as char in buffer
                        sprintf(uid_buff, "%d", uid);

                        char *argv[] = {name_buff, uid_buff, sport_buff, wport_buff, thread_buff, nullptr};

                        pid_t pid;
                        int status = posix_spawn(&pid,"./hopsanserverworker",nullptr,nullptr,argv,environ);
                        if(status == 0)
                        {
                            std::cout << PRINTSERVER << nowDateTime() << " Launched Worker Process, pid: "<< pid << " port: " << workerPort << " uid: " << uid << " nThreads: " << requestNumThreads << endl;
                            workerMap.insert({uid, WorkerInfo(requestNumThreads, workerPort, requestuserid, pid)});

                            ReplymsgReplyServerSlots msg = {int(workerPort)};
                            sendMessage(socket, ReplyServerSlots, msg);
                            gNumTakenSlots+=requestNumThreads;
                            std::cout << PRINTSERVER << nowDateTime() << " Remaining slots: " << gServerConfig.mMaxNumSlots-gNumTakenSlots << endl;
                        }
                        else
                        {
                            std::cout << PRINTSERVER << nowDateTime() << " Error: Failed to launch worker process!"<<endl;
                            sendMessage(socket, NotAck, "Failed to launch worker process!");
                        }
#endif
                    }
                    else if (gNumTakenSlots == gServerConfig.mMaxNumSlots)
                    {
                        sendMessage(socket, NotAck, "All slots taken");
                        cout << PRINTSERVER << nowDateTime() << " Denied! All slots taken." << endl;
                    }
                    else
                    {
                        sendMessage(socket, NotAck, "To few free slots");
                        cout << PRINTSERVER << nowDateTime() << " Denied! To few free slots." << endl;
                    }
                }
                else if (msg_id == WorkerFinished)
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
                            sendShortMessage(socket, Ack);

                            // Wait for process to stop (to avoid zombies)
                            waitForWorkerProcess(it->second);
//#ifdef _WIN32
//                            PROCESS_INFORMATION pi = it->second.pi;
//                            WaitForSingleObject( pi.hProcess, INFINITE );
//                            CloseHandle( pi.hProcess );
//                            CloseHandle( pi.hThread );
//#else
//                            pid_t pid = it->second.pid;
//                            int stat_loc;
//                            pid_t status = waitpid(pid, &stat_loc, WUNTRACED);
//#endif
                            //! @todo check return codes maybe
                            int nslots = it->second.mNumSlots;
                            workerMap.erase(it);
                            gNumTakenSlots -= nslots;
                            std::cout << PRINTSERVER << nowDateTime() << " Open slots: " << gServerConfig.mMaxNumSlots-gNumTakenSlots << endl;
                        }
                        else
                        {
                            sendMessage(socket, NotAck, "Wrong worker id specified");
                        }
                    }
                    else
                    {
                        cout << PRINTSERVER << nowDateTime() << " Error: Could not parse server id string" << endl;
                    }
                }
                else if (msg_id == WorkerAlive)
                {
                    bool parseOK;
                    string id_string = unpackMessage<std::string>(request,offset,parseOK);
                    if (parseOK)
                    {
                        int id = atoi(id_string.c_str());
                        cout << PRINTSERVER << nowDateTime() << " Worker " << id_string << " Alive!" << endl;
                        auto it = workerMap.find(id);
                        if (it != workerMap.end())
                        {
                            sendShortMessage(socket, Ack);
                            it->second.mLastAliveReport = steady_clock::now();
                        }
                        else
                        {
                            sendMessage(socket, NotAck, "Wrong worker id specified");
                        }
                    }
                    else
                    {
                        cout << PRINTSERVER << nowDateTime() << " Error: Could not parse server id string" << endl;
                    }
                }
                else if (msg_id == RequestServerStatus)
                {
                    cout << PRINTSERVER << nowDateTime() << " Client is requesting status" << endl;
                    ReplymsgReplyServerStatus status;
                    status.numTotalSlots = gServerConfig.mMaxNumSlots;
                    status.numFreeSlots = gServerConfig.mMaxNumSlots-gNumTakenSlots;
                    status.isReady = true;
                    for (auto it=workerMap.begin(); it!=workerMap.end(); ++it)
                    {
                        status.users += it->second.mUserid+", ";
                    }
                    if (!workerMap.empty())
                    {
                        status.users.pop_back();
                        status.users.pop_back();
                    }

                    sendMessage(socket, ReplyServerStatus, status);
                    lastStatusRequestTime = chrono::steady_clock::now();
                }
                // Ignore the following messages silently
                else if (msg_id == ClientClosing)
                {
                    sendShortMessage(socket, Ack);
                }
                else if (!idParseOK)
                {
                    cout << PRINTSERVER << nowDateTime() << " Error: Could not parse message id" << endl;
                }
                else
                {
                    stringstream ss;
                    ss << PRINTSERVER << nowDateTime() << " Warning: Unhandled message id " << msg_id << endl;
                    cout << ss.str() << endl;
                    sendMessage(socket, NotAck, ss.str());
                }
            }
            else
            {
                // Handle timeout / exception
            }

            // Check if we should report back to address server, if no status requests have been made in some time
            if (argAddressServerIP.isSet())
            {
                duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - lastStatusRequestTime);
                if (time_span.count() > gServerConfig.mAddressReportAge)
                {
                    cout << PRINTSERVER << nowDateTime() << " Too long since status check: " << time_span.count() << endl;

                    // If no one has requested status for this long (and we have a master server) we assume that the master server
                    // has gone down, lets reconnect to it to make sure it knows that we still exist
                    //! @todo maybe this should be handled in the server by saving known servers to file instead
                    reportToAddressServer(gServerConfig, true);
                }
            }

            // Go through all running workers and check if we should try to request status from them, to see if they are still alive
            //! @todo since we are not using the status data here, maybe we should use ping/pong messages instead
            for (auto it = workerMap.begin(); it!=workerMap.end(); ++it)
            {
                WorkerInfo &wi = it->second;
                if (duration_cast<duration<double>>(steady_clock::now() - wi.mLastAliveReport).count() > 10*60)
                {
                    bool isAlive = checkIfWorkerIsAlive(wi);
                    if (!isAlive)
                    {
                        cout << PRINTSERVER << nowDateTime() << "Worker: " << it->first << " is not responding!" << std::endl;
                        std::cout << PRINTSERVER << nowDateTime() << "Burying dead worker: " << it->first << endl;
                        waitForWorkerProcess(wi);
                        int nslots = wi.mNumSlots;
                        workerMap.erase(it);
                        gNumTakenSlots -= nslots ;
                        std::cout << PRINTSERVER << nowDateTime() << "Open slots: " << gServerConfig.mMaxNumSlots-gNumTakenSlots << endl;
                    }
                }
            }

            if (s_interrupted)
            {
                cout << PRINTSERVER << nowDateTime() << " Interrupt signal received, killing server" << std::endl;
                break;
            }
        }

        // Tell master server we are closing
        if (argAddressServerIP.isSet())
        {
            //! @todo somehow automatically figure out my IP
            reportToAddressServer(gServerConfig, false);
        }

    }
    catch(zmq::error_t e)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: Preparing our context and socket: " << e.what() << endl;
    }

    cout << PRINTSERVER << nowDateTime() << " Closed!" << endl;
}

