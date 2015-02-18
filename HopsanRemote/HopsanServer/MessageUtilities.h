//$Id$

#ifndef PACKANDSEND_H
#define PACKANDSEND_H

#include "Messages.h"
#include "msgpack.hpp"
#include <string>

template <typename T>
void sendServerMessage(zmq::socket_t &rSocket, ServerMessageIdEnumT id, const T &rMessage)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rMessage);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}


void sendServerStringMessage(zmq::socket_t &rSocket, ServerMessageIdEnumT id, const std::string &rString)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rString);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}

void sendServerAck(zmq::socket_t &rSocket)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, S_Ack);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}

inline void sendServerNAck(zmq::socket_t &rSocket, const std::string &rReason)
{
    sendServerStringMessage(rSocket, S_NAck, rReason);
}

bool receiveWithTimeout(zmq::socket_t &rSocket, long timeout, zmq::message_t &rMessage)
{
    // Create a poll item
    zmq::pollitem_t pollitems[] = {{ rSocket, 0, ZMQ_POLLIN, 0 }};
    try
    {
        // Poll socket for a reply, with timeout
        zmq::poll(&pollitems[0], 1,  timeout);

        // If we have received a message then read message and return true
        if (pollitems[0].revents & ZMQ_POLLIN)
        {
            rSocket.recv(&rMessage);
            return true;
        }
        // Else we reached timeout, return false
    }
    catch(zmq::error_t e)
    {
        //Ignore
    }
    return false;
}

template <typename T>
inline T unpackMessage(zmq::message_t &rRequest, size_t &rOffset)
{
    return msgpack::unpack(static_cast<char*>(rRequest.data()), rRequest.size(), rOffset).get().as<T>();
}

//inline size_t parseMessageId(char* pBuffer, size_t len, size_t &rOffset)
//{
//    return msgpack::unpack(pBuffer, len, rOffset).get().as<size_t>();
//}

inline size_t getMessageId(zmq::message_t &rMsg, size_t &rOffset)
{
    //return parseMessageId(static_cast<char*>(rMsg.data()), rMsg.size(), rOffset);
    return msgpack::unpack(static_cast<char*>(rMsg.data()), rMsg.size(), rOffset).get().as<size_t>();
}

inline std::string makeZMQAddress(std::string ip, size_t port)
{
    return "tcp://" + ip + ":" + std::to_string(port);
}

inline std::string makeZMQAddress(std::string ip, std::string port)
{
    return "tcp://" + ip + ":" + port;
}

#endif // PACKANDSEND_H
