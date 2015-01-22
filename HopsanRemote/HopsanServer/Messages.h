//$Id$

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <sstream>
#include "msgpack.hpp"

inline size_t parseMessageId(char* pBuffer, size_t len, size_t &rOffset)
{
    return msgpack::unpack(pBuffer, len, rOffset).get().as<size_t>();
}

inline std::string makeZMQAddress(std::string ip, size_t port)
{
    return "tcp://" + ip + ":" + std::to_string(port);
}

enum ClientMessageIdEnumT {C_Ack, C_NAck, C_Bye, C_ReqSlot, C_SendingHmf, C_SetParam, C_GetParam, C_Simulate, C_ReqResults, C_ReqMessages};
enum ServerMessageIdEnumT {S_Ack=128, S_NAck, S_ReqSlot_Reply, S_GetParam_Reply, S_ReqResults_Reply, S_ReqMessages_Reply};

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




// Server messages

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

#endif // MESSAGES_H
