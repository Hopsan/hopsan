//$Id$

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include "ServerStatusMessage.h"
#include "msgpack.hpp"

enum ClientMessageIdEnumT {C_Ack=1,
                           C_NAck,
                           C_Bye,
                           C_ReqServerStatus,
                           C_ReqSlot,
                           C_SendingHmf,
                           C_SetParam,
                           C_GetParam,
                           C_Simulate,
                           C_ReqResults,
                           C_ReqMessages,
                           C_ReqServerMachines,
                           C_ReqWorkerStatus,
                           C_ReqBenchmark,
                           C_ReqBenchmarkResults,
                           C_Abort};
enum ServerMessageIdEnumT {S_Ack=128,
                           S_NAck,
                           S_Available,
                           S_Closing,
                           S_ReqServerStatus_Reply,
                           S_ReqServerMachines_Reply,
                           S_ReqSlot_Reply,
                           S_ReqBenchmarkResults_Reply,
                           SW_GetParam_Reply,
                           SW_ReqWorkerStatus_Reply,
                           SW_ReqResults_Reply,
                           SW_ReqMessages_Reply,
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
    std::string model;
    MSGPACK_DEFINE(model)
}CM_ReqBenchmark_t;

typedef struct
{
    int numMachines;
    int numThreads;
    double maxBenchmarkTime;
    MSGPACK_DEFINE(numMachines, numThreads, maxBenchmarkTime)
}CM_ReqServerMachines_t;

typedef struct
{
    int numThreads;
    MSGPACK_DEFINE(numThreads)
}CM_ReqSlot_t;

// Server messages

typedef struct
{
    std::string ip;
    std::string port;
    std::string description;
    MSGPACK_DEFINE(ip,port,description)
}SM_Available_t;

typedef struct
{
    size_t port;
    MSGPACK_DEFINE(port)
}SM_ReqSlot_Reply_t;

typedef struct SM_ServerStatus_ : ServerStatusT
{
    MSGPACK_DEFINE(numFreeSlots, numTotalSlots, startTime, stopTime, isReady)
}SM_ServerStatus_t;

typedef struct
{
    int numthreads;
    double inittime;
    double simutime;
    double finitime;
    MSGPACK_DEFINE(numthreads, inittime, simutime, finitime)
}SWM_ReqBenchmarkResults_Reply_t;

// Worker messages

typedef struct
{
    std::string value;
    std::string unit;
    MSGPACK_DEFINE(value,unit)
}SM_GetParam_Reply_t;

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

typedef struct SWM_ReqWorkerStatus_Reply_ : WorkerStatusT
{
    MSGPACK_DEFINE(model_loaded, simulation_inprogress, simualtion_success, simulation_finished,
                   current_simulation_time, simulation_progress, estimated_simulation_time_remaining)
}SWM_ReqWorkerStatus_Reply_t;

// Address server messages

typedef struct
{
    std::vector<std::string> ips;
    std::vector<std::string> ports;
    std::vector<std::string> descriptions;
    std::vector<int> numslots;
    std::vector<double> speeds;     //! @todo calling it speed is bad, its the simtime lower is better (evaluationTime better name)
    MSGPACK_DEFINE(ips, ports, descriptions, numslots, speeds)
}MSM_ReqServerMachines_Reply_t;



#endif // MESSAGES_H
