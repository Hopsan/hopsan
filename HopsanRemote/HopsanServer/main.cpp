//$Id$

#include <streambuf>
#include <sstream>
#include <cstdlib>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <iostream>
//#include <thread>
#include <vector>
#include "zmq.hpp"
#include <map>

#include "Messages.h"
#include "MessageUtilities.h"

#ifdef _WIN32
//#include <strsafe.h>
#include <tchar.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#endif

using namespace std;

#ifdef _WIN32
map<int, PROCESS_INFORMATION> workerMap;
#else
map<int, pid_t> workerMap;
#endif

extern char **environ;

class ServerConfig
{
public:
    int mControlPort = 23300;
    int mClientPortStart = 23301;
    int mClientPortEnd = 23310;
    int mMaxClients = 20;
    int mMaxThreadsPerClient = 2;
};

ServerConfig gServerConfig;
size_t nTakenSlots=0;

#define PRINTSERVER "Server; "

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << PRINTSERVER << "Error: you must specify what base port to use!" << endl;
        return 1;
    }

    cout << PRINTSERVER << "Listening on port: " << argv[1]  << endl;

    // Prepare our context and socket
#ifdef _WIN32
    zmq::context_t context(1, 63);
#else
    zmq::context_t context(1);
#endif
    zmq::socket_t socket (context, ZMQ_REP);

    socket.bind( makeZMQAddress("*", argv[1]).c_str() );

    while (true) {
        zmq::message_t request;

        // Wait for next request from client
        socket.recv (&request);
        size_t offset=0;
        size_t msg_id = getMessageId(request, offset);
//        cout << PRINTSERVER << "Received message with length: " << request.size() << " msg_id: " << msg_id << endl;

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
            cout << PRINTSERVER << "Client is requesting slot... " << endl;
            msgpack::v1::sbuffer out_buffer;
            if (nTakenSlots < gServerConfig.mMaxClients)
            {
                size_t port = gServerConfig.mControlPort+1+nTakenSlots;

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
                string swport = to_string(port);
                string nthreads = to_string(gServerConfig.mMaxThreadsPerClient);
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
                    workerMap.insert({uid,processInformation});

                    SM_ReqSlot_Reply_t msg = {port};
                    msgpack::pack(out_buffer, S_ReqSlot_Reply);
                    msgpack::pack(out_buffer, msg);
                    socket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
                    nTakenSlots++;
                }

#else
                char sport_buff[64], wport_buff[64], thread_buff[64], uid_buff[64];
                // Write port as char in buffer
                sprintf(sport_buff, "%d", gServerConfig.mControlPort);
                sprintf(wport_buff, "%d", int(port));
                // Write num threads as char in buffer
                sprintf(thread_buff, "%d", gServerConfig.mMaxThreadsPerClient);
                // Write id as char in buffer
                sprintf(uid_buff, "%d", uid);

                char *argv[] = {"HopsanServerWorker", uid_buff, sport_buff, wport_buff, thread_buff, nullptr};

                pid_t pid;
                int status = posix_spawn(&pid,"./HopsanServerWorker",nullptr,nullptr,argv,environ);
                if(status == 0)
                {
                    std::cout << PRINTSERVER << "Launched Worker Process, pid: "<< pid << " port: " << port << " uid: " << uid << endl;
                    workerMap.insert({uid,pid});

                    SM_ReqSlot_Reply_t msg = {port};
                    msgpack::pack(out_buffer, S_ReqSlot_Reply);
                    msgpack::pack(out_buffer, msg);
                    socket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
                    nTakenSlots++;

                }
                else
                {
                    std::cout << PRINTSERVER << "Error: Failed to launch worker process!"<<endl;
                    sendServerNAck(socket, "Failed to launch worker process!");
                }
#endif
            }
            else
            {
                sendServerNAck(socket, "All slots taken");
                cout << PRINTSERVER << "Denied! All slots taken." << endl;
            }

        }
        else if (msg_id == SW_Finished)
        {
            string id_string = unpackMessage<std::string>(request,offset);
            int id = atoi(id_string.c_str());
            cout << PRINTSERVER << "Worker " << id_string << " Finished!" << endl;

            auto it = workerMap.find(id);
            if (it != workerMap.end())
            {
                sendServerAck(socket);

#ifdef _WIN32
                PROCESS_INFORMATION pi = it->second;
                WaitForSingleObject( pi.hProcess, INFINITE );
                CloseHandle( pi.hProcess );
                CloseHandle( pi.hThread );
#else
                // Wait for process to stop (to avoid zombies)
                pid_t pid = it->second;
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
            stringstream ss;
            ss << PRINTSERVER << "Error: Unknown message id " << msg_id << endl;
            cout << ss.str() << endl;
            sendServerNAck(socket, ss.str());
        }

        // Do some 'work'
#ifndef _WIN32
        //sleep(1);
#else
        //Sleep (1);
#endif
    }
}

