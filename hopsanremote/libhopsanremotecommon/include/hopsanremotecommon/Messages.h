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

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include "StatusInfoStructs.h"
#include "DataStructs.h"
#include "msgpack.hpp"

enum MessageIdsEnumT {
    /* Information messages */
    Ack=1,
    NotAck,
    ServerAvailable,
    WorkerFinished,
    ClientClosing,
    ServerClosing,

    /* Command messages */
    Abort,
    Simulate,
    Benchmark,
    SetParameter,
    SetModel,
    SendFile,
    RequestFile,
    ReleaseRelaySlot,
    ExecuteInShell,
    IdentifyUser,

    /* Request messages */
    RequestServerMachines,
    RequestServerStatus,
    RequestBenchmarkResults,
    RequestServerSlots,
    RequestWorkerStatus,
    RequestParameter,
    RequestResults,
    RequestMessages,
    RequestShellOutput,
    RequestRelaySlot,

    /* Reply messages (to requests) */
    ReplyServerMachines,
    ReplyServerStatus,
    ReplyBenchmarkResults,
    ReplyServerSlots,
    ReplyWorkerStatus,
    ReplyParameter,
    ReplyResults,
    ReplyMessages,
    ReplyShellOutput,
    ReplyRelaySlot,

    /* Work in progress (last to avoid breaking compatibility */
    WorkerAlive,

};

MSGPACK_ADD_ENUM(MessageIdsEnumT)

// Message structures for messages typically used by Clients

class CmdmsgSetParameter
{
public:
    std::string name;
    std::string value;

    MSGPACK_DEFINE(name, value)
};

class CmdmsgSimulate
{
public:
    int nLogSamples = -1;
    int logStartTim = -1;
    int simStartTime = -1;
    int simTimestep = -1;
    int simStopTime = -1;

    MSGPACK_DEFINE(nLogSamples, logStartTim, simStartTime, simTimestep, simStopTime)
};

class CmdmsgBenchmark
{
public:
    std::string model;

    MSGPACK_DEFINE(model)
};

class ReqmsgRequestParameter
{
public:
    std::string name;

    MSGPACK_DEFINE(name)
};

class ReqmsgRequestServerMachines
{
public:
    int numMachines;
    int numThreads;
    double maxBenchmarkTime;

    MSGPACK_DEFINE(numMachines, numThreads, maxBenchmarkTime)
};

class ReqmsgReqServerSlots
{
public:
    int numThreads;
    std::string userid;

    MSGPACK_DEFINE(numThreads, userid)
};

class ReqmsgRelaySlot
{
public:
    std::string relaybaseid;
    int ctrlport;

    MSGPACK_DEFINE(relaybaseid, ctrlport)
};

class CmdmsgSendFile
{
public:
    std::string filename;
    std::string data;
    bool islastpart;

    MSGPACK_DEFINE(filename, data, islastpart)
};

class CmdmsgRequestFile
{
public:
    std::string filename;
    int offset;
    MSGPACK_DEFINE(filename, offset)
};

class CmdmsgIdentifyUser
{
public:
    std::string username;
    std::string password;
    MSGPACK_DEFINE(username, password)
};


// Message structures for messages typically used by Servers

class InfomsgAvailable
{
public:
    std::string ip;
    std::string port;
    std::string description;
    std::string services;
    int numTotalSlots;
    int identity;

    MSGPACK_DEFINE(ip,port,description,services,numTotalSlots,identity)
};

class ReplymsgReplyServerSlots
{
public:
    int portoffset;

    MSGPACK_DEFINE(portoffset)
};

class ReplymsgReplyServerStatus : public ServerStatusT
{
public:
    MSGPACK_DEFINE(services, users, numFreeSlots, numTotalSlots, startTime, stopTime, isReady)
};

class ReplymsgReplyBenchmarkResults
{
public:
    int numthreads;
    double inittime;
    double simutime;
    double finitime;

    MSGPACK_DEFINE(numthreads, inittime, simutime, finitime)
};


// Message structs typically used by the Workers

class ReplymsgReplyParameter
{
public:
    std::string value;
    std::string unit;

    MSGPACK_DEFINE(value,unit)
};

class ReplymsgResultsVariable : public ResultVariableT
{
public:
    MSGPACK_DEFINE(name,alias,quantity,unit,data)
};

class ReplymsgReplyMessage
{
public:
    char type;
    std::string message;
    std::string tag;

    MSGPACK_DEFINE(type,message,tag)
};

class ReplymsgReplyWorkerStatus : public WorkerStatusT
{
public:
    MSGPACK_DEFINE(model_loaded, simulation_inprogress, simualtion_success, simulation_finished,
                   current_simulation_time, simulation_progress, estimated_simulation_time_remaining,
                   shell_inprogress, shell_exitok)
};

// Message structs typically used by the Adress server

class ReplymsgReplyServerMachine : public ServerMachineInfoT
{
public:
    MSGPACK_DEFINE(address, relayaddress, description, numslots, evalTime)
};



#endif // MESSAGES_H
