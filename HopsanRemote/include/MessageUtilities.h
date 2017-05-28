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

#ifndef PACKANDSEND_H
#define PACKANDSEND_H

#include "Messages.h"
#include "msgpack.hpp"
#include "zmq.hpp"
#include <string>
#include <iostream>


inline bool receiveWithTimeout(zmq::socket_t &rSocket, long timeout, zmq::message_t &rMessage)
{
    // Create a poll item
    std::vector<zmq::pollitem_t> pollitems {{ (void*)rSocket, 0, ZMQ_POLLIN, 0 }};
    try
    {
        // Poll socket for a reply, with timeout
        zmq::poll(pollitems,  timeout);

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
    // Catch an unpack error
    catch( msgpack::unpack_error e)
    {
        std::cout << "EXCEPTION in unpackMessage: " << e.what() << std::endl;
        rUnpackOK = false;
        return T();
    }
    // Catch any other exception
    catch( std::exception e)
    {
        std::cout << "EXCEPTION in unpackMessage: " << e.what() << std::endl;
        rUnpackOK = false;
        return T();
    }
}

inline size_t getMessageId(zmq::message_t &rMsg, size_t &rOffset, bool &rUnpackOK)
{
    return unpackMessage<size_t>(rMsg, rOffset, rUnpackOK);
}

inline
void sendShortMessage(zmq::socket_t &rSocket, MessageIdsEnumT id)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}

template <typename T>
inline
zmq::message_t createZmqMessage(MessageIdsEnumT id, const T &rMessage)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rMessage);

    zmq::message_t msg(out_buffer.size());
    memcpy(msg.data(), static_cast<void*>(out_buffer.data()), out_buffer.size());
    return msg;
}

template <typename T>
inline
void sendMessage(zmq::socket_t &rSocket, MessageIdsEnumT id, const T &rMessage)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rMessage);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}

inline
bool sendIdentityEnvelope(zmq::socket_t &rSocket, const std::string &rIdentity)
{
    zmq::message_t message(rIdentity.size());
    memcpy (message.data(), rIdentity.data(), rIdentity.size());
    bool rc = rSocket.send (message, ZMQ_SNDMORE);
    if (rc)
    {
        zmq::message_t empty(0);
        rc = rSocket.send (empty, ZMQ_SNDMORE);
    }
    return (rc);
}

inline std::string makeZMQAddress(const std::string &ipwithport)
{
    return "tcp://" + ipwithport;
}

inline std::string makeZMQAddress(const std::string &ip, const std::string &port)
{
    return makeZMQAddress(ip+":"+port);
}

inline std::string makeZMQAddress(const std::string &ip, int port)
{
    return makeZMQAddress(ip, std::to_string(port));
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

inline
std::vector<std::string> splitstring(const std::string &str, const std::string &delim)
{
    std::vector<std::string> out;
    size_t b=0, e, n;
    bool exit=false;
    do
    {
        e = str.find_first_of(delim, b);
        if (e == std::string::npos)
        {
            n = e;
            exit=true;
        }
        else
        {
            n = e-b;
        }
        out.push_back(str.substr(b,n));
        b = e+1;
    }while(!exit);
    return out;
}

inline
void splitaddress(const std::string &rStr, std::string &rIp, std::string &rPort, std::string &rRelayId)
{
    std::vector<std::string> fields = splitstring(rStr, ":");
    if (fields.size() >= 2)
    {
        rIp = fields[0];
        rPort = fields[1];
    }
    if (fields.size() >= 3)
    {
        rRelayId = fields[2];
    }
}

inline
void splitaddressandrelayid(const std::string &rStr, std::string &rIp, std::string &rPort, std::string &rFullRelayId, std::string &rBaseRelayId, std::string &rSubRelayId)
{
    splitaddress(rStr, rIp, rPort, rFullRelayId);
    std::vector<std::string> fields = splitstring(rFullRelayId, ".");
    if (fields.size() == 2)
    {
        rBaseRelayId = fields[0];
        rSubRelayId = fields[1];
    }
    else
    {
        rBaseRelayId = rFullRelayId;
        rSubRelayId.clear();
    }
}

inline
std::string getRealyId(const std::string &rAddress)
{
    std::vector<std::string> fields = splitstring(rAddress, ":");
    if (fields.size() >= 3)
    {
        return fields[2];
    }
    return "";
}

#endif // PACKANDSEND_H
