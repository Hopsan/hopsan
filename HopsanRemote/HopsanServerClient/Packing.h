//$Id$

#ifndef PACKING_H
#define PACKING_H

#include "msgpack.hpp"
#include "Messages.h"

template <typename T>
void sendMessage(zmq::socket_t &rSocket, ClientMessageIdEnumT id, const T &rMessage)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rMessage);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}


template <typename T>
inline T unpackMessage(zmq::message_t &rResponse, size_t &rOffset)
{
    return msgpack::unpack(static_cast<char*>(rResponse.data()), rResponse.size(), rOffset).get().as<T>();
}

#endif // PACKING_H
