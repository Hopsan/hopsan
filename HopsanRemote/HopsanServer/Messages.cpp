#include "Messages.h"
#include "msgpack.hpp"

#include <iostream>

using namespace std;

//setParamMessage_t &&parseSetParamMessage(char *pBuffer, size_t len, size_t &rOffset)
//{
//    setParamMessage_t msg;
//    msgpack::unpacked name = msgpack::unpack(pBuffer, len, rOffset);

//}


//setParamMessage_t unpackSetParamMessage(char *pBuffer, size_t len, size_t &rOffset)
//{
//    setParamMessage_t msg;
//    cout << "Server: " << rOffset << " " << len << endl;
//    msg.name = msgpack::unpack(pBuffer, len, rOffset).get().as<std::string>();
//    cout << "Server: " << rOffset << " " << len << endl;
//    msg.value = msgpack::unpack(pBuffer, len, rOffset).get().as<double>();
//    cout << "Server: " << rOffset << " " << len << endl;
//    assert(len == rOffset);
//    return msg;
//}


//void packSetParamMessage(std::string &&rName, double value, msgpack::v1::sbuffer &rBuff)
//{
//    msgpack::pack(rBuff, C_SET_PARAM);
//    msgpack::pack(rBuff, rName );
//    msgpack::pack(rBuff, value );
//}


//size_t parseMessageId(char *pBuffer, size_t len, size_t &rOffset)
//{

//}


//void packServerAckMessage(msgpack::v1::sbuffer &rBuff)
//{
//    msgpack::pack(rBuff, S_ACK);
//}


//void packServerNAckMessage(msgpack::v1::sbuffer &rBuff)
//{
//    msgpack::pack(rBuff, S_NACK);
//}


