//$Id$

#ifndef PACKANDSEND_H
#define PACKANDSEND_H

#include "Messages.h"
#include "msgpack.hpp"
#include <string>
#include <iostream>


inline bool receiveWithTimeout(zmq::socket_t &rSocket, long timeout, zmq::message_t &rMessage)
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
        std::cout << "EXCEPTION in receiveWithTimeout: " << e.what() << std::endl;
    }
    return false;
}

template <typename T>
inline T unpackMessage(zmq::message_t &rRequest, size_t &rOffset, bool &rUnpackOK)
{
    try
    {
        rUnpackOK = true;
        //! @todo if extra bytes and some other conditions, unpack will not throw an exception and as<T>() will krash the program with SIG ABORT
        return msgpack::unpack(static_cast<char*>(rRequest.data()), rRequest.size(), rOffset).get().as<T>();
    }
    catch( msgpack::unpack_error e)
    {
        std::cout << "EXCEPTION in unpackMessage: " << e.what() << std::endl;
        rUnpackOK = false;
        return T();
    }
}

//inline size_t parseMessageId(char* pBuffer, size_t len, size_t &rOffset)
//{
//    return msgpack::unpack(pBuffer, len, rOffset).get().as<size_t>();
//}

inline size_t getMessageId(zmq::message_t &rMsg, size_t &rOffset, bool &rUnpackOK)
{
    //return parseMessageId(static_cast<char*>(rMsg.data()), rMsg.size(), rOffset);
    //return msgpack::unpack(static_cast<char*>(rMsg.data()), rMsg.size(), rOffset).get().as<size_t>();
    return unpackMessage<size_t>(rMsg, rOffset, rUnpackOK);
}

inline std::string makeZMQAddress(std::string ip, size_t port)
{
    return "tcp://" + ip + ":" + std::to_string(port);
}

inline std::string makeZMQAddress(std::string ip, std::string port)
{
    return "tcp://" + ip + ":" + port;
}

inline void splitZMQAddress(const std::string &rZMQAddress, std::string &rProtocol, std::string &rIP, std::string &rPort)
{
    size_t pe = rZMQAddress.find_last_of('/');
    if (pe != std::string::npos)
    {
        rProtocol = rZMQAddress.substr(0, pe-2);
        size_t ipe = rZMQAddress.find_last_of(':');
        if (ipe != std::string::npos)
        {
            rIP = rZMQAddress.substr(pe+1, ipe-pe-1);
            rPort = rZMQAddress.substr(ipe+1);
        }
    }
}

#endif // PACKANDSEND_H
