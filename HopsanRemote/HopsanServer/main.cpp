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

#include "Messages.h"
#include "PackAndSend.h"
#include "global.h"

#include <spawn.h>

using namespace std;

extern char **environ;

ServerConfig gServerConfig;

size_t nTakenSlots=0;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Error: you must specify what base port to use!" << endl;
        return 1;
    }

    cout << "Server Process Starting with base port: " << argv[1]  << endl;

    // Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    cout << makeZMQAddress("*", size_t(atoi(argv[1]))).c_str() << endl;
    socket.bind( makeZMQAddress("*", size_t(atoi(argv[1]))).c_str() );

    while (true) {
        zmq::message_t request;

        // Wait for next request from client
        socket.recv (&request);
        size_t offset=0;
        size_t msg_id = parseMessageId(static_cast<char*>(request.data()), request.size(), offset);
        cout << "Server received message with length: " << request.size() << " msg_id: " << msg_id << endl;
        if (msg_id == C_ReqSlot)
        {
            cout << "Client is requesting slot... " << endl;
            msgpack::v1::sbuffer out_buffer;
            if (nTakenSlots < gServerConfig.mMaxClients)
            {
                size_t port = gServerConfig.mControlPort+1+nTakenSlots;
                char buff[64];
                sprintf(buff,"%d", int(port));

                char *argv[] = {"HopsanServerWorker", buff, nullptr};
                pid_t pid;
                cout << "argv: " << argv[0] << " " << argv[1] << endl;
                int status = posix_spawn(&pid,"/home/petno25/svn/hopsan/trunk/HopsanRemote/bin/HopsanServerWorker",nullptr,nullptr,argv,environ);
                if(status == 0)
                {
                    std::cout<<"Launched Worker Process, pid: "<< pid << " port: " << port << endl;

                    SM_ReqSlot_Reply_t msg = {port};
                    msgpack::pack(out_buffer, S_ReqSlot_Reply);
                    msgpack::pack(out_buffer, msg);
                    socket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
                    nTakenSlots++;

                }
                else
                {
                    std::cout<<"Failed to launch worker process!"<<endl;
                    sendServerNAck(socket, "Failed to launch worker process!");
                }
            }
            else
            {
                sendServerNAck(socket, "All slots taken");
                cout << "Denied!" << endl;
            }
        }
        else if (msg_id == C_Bye)
        {
            cout << "Client said godbye!" << endl;
            nTakenSlots--;
            sendServerAck(socket);
        }
        else
        {
            stringstream ss;
            ss << "Server error: Unknown message id " << msg_id << endl;
            cout << "Server error: Unknown message id " << msg_id << endl;
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

