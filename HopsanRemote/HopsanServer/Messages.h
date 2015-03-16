//$Id$

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include "ServerStatusMessage.h"
#include "msgpack.hpp"

enum ClientMessageIdEnumT {C_Ack=1,
                           C_NAck,
                           C_Bye,
                           C_ReqAlive,
                           C_ReqStatus,
                           C_ReqSlot,
                           C_SendingHmf,
                           C_SetParam,
                           C_GetParam,
                           C_Simulate,
                           C_ReqResults,
                           C_ReqMessages,
                           C_ReqServerMachines};
enum ServerMessageIdEnumT {S_Ack=128,
                           S_NAck,
                           S_Available,
                           S_Closing,
                           S_ReqStatus_Reply,
                           S_ReqSlot_Reply,
                           S_GetParam_Reply,
                           S_ReqResults_Reply,
                           S_ReqMessages_Reply,
                           S_ReqServerMachines_Reply,
                           SW_Finished};

MSGPACK_ADD_ENUM(ClientMessageIdEnumT)
MSGPACK_ADD_ENUM(ServerMessageIdEnumT)

// Client messages

typedef struct
{
    std::string name;
    std::string value;
    MSGPACK_DEFINE(name, value)
}CM_SetParam_t;

typedef struct
{
    std::string name;
    MSGPACK_DEFINE(name)
}CM_GetParam_t;

typedef struct
{
    int nLogSamples = -1;
    int logStartTim = -1;
    int simStartTime = -1;
    int simTimestep = -1;
    int simStopTime = -1;
    MSGPACK_DEFINE(nLogSamples, logStartTim, simStartTime, simTimestep, simStopTime)
}CM_Simulate_t;

typedef struct
{
    int numMachines;
    int numThreads;
    double maxBenchmarkTime;
    MSGPACK_DEFINE(numMachines, numThreads, maxBenchmarkTime)
}CM_ReqServerMachines_t;


// Server messages

typedef struct
{
    std::string ip;
    std::string port;
    MSGPACK_DEFINE(ip,port)
}SM_Available_t;

typedef struct
{
    std::string value;
    std::string unit;
    MSGPACK_DEFINE(value,unit)
}SM_GetParam_Reply_t;

typedef struct
{
    size_t port;
    MSGPACK_DEFINE(port)
}SM_ReqSlot_Reply_t;

typedef struct
{
    std::string name;
    std::string alias;
    std::string unit;
    std::vector<double> data;
    MSGPACK_DEFINE(name,alias,unit,data)
}SM_Variable_Description_t;

typedef struct
{
    char type;
    std::string message;
    std::string tag;
    MSGPACK_DEFINE(type,message,tag)
}SM_HopsanCoreMessage_t;

typedef struct SM_ServerStatus_ : ServerStatusT
{
    MSGPACK_DEFINE(numFreeSlots, numTotalSlots, numThreadsPerSlot, startTime, stopTime, isReady)
}SM_ServerStatus_t;

typedef struct
{
    std::vector<std::string> ips;
    std::vector<std::string> ports;
    MSGPACK_DEFINE(ips, ports)
}MSM_ReqServerMachines_Reply_t;

#endif // MESSAGES_H
