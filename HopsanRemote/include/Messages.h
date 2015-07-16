//$Id$

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include "StatusInfoStructs.h"
#include "msgpack.hpp"

enum MessageIdsEnumT {
    /* Information messages */
    Ack=1,
    NotAck,
    Available,
    Finished,
    Closing,

    /* Command messages */
    Abort,
    Simulate,
    Benchmark,
    SetParameter,
    SetModel,

    /* Request messages */
    RequestServerMachines,
    RequestServerStatus,
    RequestBenchmarkResults,
    RequestServerSlots,
    RequestWorkerStatus,
    RequestParameter,
    RequestResults,
    RequestMessages,

    /* Reply messages (to requests) */
    ReplyServerMachines,
    ReplyServerStatus,
    ReplyBenchmarkResults,
    ReplyServerSlots,
    ReplyWorkerStatus,
    ReplyParameter,
    ReplyResults,
    ReplyMessages};

MSGPACK_ADD_ENUM(MessageIdsEnumT)

// Message structures for messages typically used by Clients

typedef struct
{
    std::string name;
    std::string value;
    MSGPACK_DEFINE(name, value)
}cmdmsg_SetParameter_t;

typedef struct
{
    int nLogSamples = -1;
    int logStartTim = -1;
    int simStartTime = -1;
    int simTimestep = -1;
    int simStopTime = -1;
    MSGPACK_DEFINE(nLogSamples, logStartTim, simStartTime, simTimestep, simStopTime)
}cmdmsg_Simulate_t;

typedef struct
{
    std::string model;
    MSGPACK_DEFINE(model)
}cmdmsg_Benchmark_t;

typedef struct
{
    std::string name;
    MSGPACK_DEFINE(name)
}reqmsg_RequestParameter_t;

typedef struct
{
    int numMachines;
    int numThreads;
    double maxBenchmarkTime;
    MSGPACK_DEFINE(numMachines, numThreads, maxBenchmarkTime)
}reqmsg_RequestServerMachines_t;

typedef struct
{
    int numThreads;
    MSGPACK_DEFINE(numThreads)
}reqmsg_ReqServerSlots_t;

// Message structures for messages typically used by Servers

typedef struct
{
    std::string ip;
    std::string port;
    std::string description;
    MSGPACK_DEFINE(ip,port,description)
}infomsg_Available_t;

typedef struct
{
    size_t port;
    MSGPACK_DEFINE(port)
}replymsg_ReplyServerSlots_t;

typedef struct replymsg_ReplyServerStatus_ : ServerStatusT
{
    MSGPACK_DEFINE(services, numFreeSlots, numTotalSlots, startTime, stopTime, isReady)
}replymsg_ReplyServerStatus_t;

typedef struct
{
    int numthreads;
    double inittime;
    double simutime;
    double finitime;
    MSGPACK_DEFINE(numthreads, inittime, simutime, finitime)
}replymsg_ReplyBenchmarkResults_t;


// Message structs typically used be the Workers

typedef struct
{
    std::string value;
    std::string unit;
    MSGPACK_DEFINE(value,unit)
}replymsg_ReplyParameter_t;

typedef struct
{
    std::string name;
    std::string alias;
    std::string unit;
    std::vector<double> data;
    MSGPACK_DEFINE(name,alias,unit,data)
}replymsg_ResultsVariable_t;

typedef struct
{
    char type;
    std::string message;
    std::string tag;
    MSGPACK_DEFINE(type,message,tag)
}replymsg_ReplyMessage_t;

typedef struct replymsg_ReplyWorkerStatus_ : WorkerStatusT
{
    MSGPACK_DEFINE(model_loaded, simulation_inprogress, simualtion_success, simulation_finished,
                   current_simulation_time, simulation_progress, estimated_simulation_time_remaining)
}replymsg_ReplyWorkerStatus_t;

// Message structs typically used be the Adress server

typedef struct
{
    std::vector<std::string> ips;
    std::vector<std::string> ports;
    std::vector<std::string> descriptions;
    std::vector<int> numslots;
    std::vector<double> evalTime;
    MSGPACK_DEFINE(ips, ports, descriptions, numslots, evalTime)
}replymsg_ReplyServerMachines_t;



#endif // MESSAGES_H
