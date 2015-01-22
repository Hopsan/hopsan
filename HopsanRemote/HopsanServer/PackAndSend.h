#ifndef PACKANDSEND_H
#define PACKANDSEND_H

#include "Messages.h"
#include "msgpack.hpp"
#include <string>

template <typename T>
void sendMessage(zmq::socket_t &rSocket, ServerMessageIdEnumT id, const T &rMessage)
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
    //cout << "ackmsg: " << out_buffer.size() << endl;
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}

inline void sendServerNAck(zmq::socket_t &rSocket, const std::string &rReason)
{
    sendServerStringMessage(rSocket, S_NAck, rReason);
}

template <typename T>
inline T unpackMessage(zmq::message_t &rRequest, size_t &rOffset)
{
    return msgpack::unpack(static_cast<char*>(rRequest.data()), rRequest.size(), rOffset).get().as<T>();
}

#endif // PACKANDSEND_H
