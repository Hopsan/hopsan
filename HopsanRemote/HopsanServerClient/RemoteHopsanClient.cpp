//$Id$

#include "RemoteHopsanClient.h"

#include "msgpack.hpp"
#include "Messages.h"
#include "Packing.h"

#include <iostream>

using namespace std;


void sendShortMessage(zmq::socket_t &rSocket, ClientMessageIdEnumT id)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}



size_t getMessageId(zmq::message_t &rMsg, size_t &rOffset)
{
    return parseMessageId(static_cast<char*>(rMsg.data()), rMsg.size(), rOffset);
}


bool readAckNackServerMessage(zmq::socket_t &rSocket, string &rNackReason)
{
    zmq::message_t response;
    rSocket.recv(&response);
    size_t offset=0;
    size_t id = getMessageId(response, offset);
    //cout << "id: " << id << endl;
    if (id == S_Ack)
    {
        return true;
    }
    else if (id == S_NAck)
    {
        rNackReason = unpackMessage<std::string>(response, offset);
    }
    else
    {
        rNackReason = "Got neither Ack nor Nack";
    }
    return false;
}

bool readAckNackServerMessage(zmq::socket_t &rSocket)
{
    string dummy;
    return readAckNackServerMessage(rSocket, dummy);
}







void RemoteHopsanClient::connectToServer(std::string addres)
{
    mRSCSocket.connect(addres.c_str());
}

bool RemoteHopsanClient::serverConnected()
{
    return mRSCSocket.connected();
}


bool RemoteHopsanClient::sendGetParamMessage(const string &rName, string &rValue)
{
    CM_GetParam_t msg {rName};
    sendMessage<CM_GetParam_t>(mRWCSocket, C_GetParam, msg);

    zmq::message_t response;
    mRWCSocket.recv(&response);
    size_t offset=0;
    size_t id = getMessageId(response, offset);
    if (id == S_GetParam_Reply)
    {
        rValue = unpackMessage<string>(response, offset);
        assert(response.size() == offset);
        return true;
    }
    else
    {
        rValue.clear();
        return false;
    }
}

bool RemoteHopsanClient::sendSetParamMessage(const string &rName, const string &rValue)
{
    CM_SetParam_t msg {rName, rValue};
    sendMessage<CM_SetParam_t>(mRWCSocket, C_SetParam, msg);

    string err;
    bool rc = readAckNackServerMessage(mRWCSocket, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::sendModelMessage(const std::string &rModel)
{
    sendMessage<std::string>(mRWCSocket, C_SendingHmf, rModel);
    string err;
    bool rc = readAckNackServerMessage(mRWCSocket, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::sendSimulateMessage(const int nLogsamples, const int logStartTime,
                         const int simStarttime, const int simSteptime, const int simStoptime)
{
    CM_Simulate_t msg;// {nLogsamples, logStartTime, simStarttime, simSteptime, simStoptime};
    sendMessage<CM_Simulate_t>(mRWCSocket, C_Simulate, msg);
    string err;
    bool rc = readAckNackServerMessage(mRWCSocket, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::requestSimulationResults(vector<string> &rDataNames, vector<double> &rData)
{
    sendMessage<string>(mRWCSocket, C_ReqResults, "*"); // Request all
    //string err;
    //return readAckNackServerMessage(rSocket,err);
    zmq::message_t response;
    mRWCSocket.recv(&response);
    cout << "Response size: " << response.size() << endl;
    size_t offset=0;
    size_t id = getMessageId(response, offset);
    if (id == S_ReqResults_Reply)
    {
        vector<SM_Variable_Description_t> vars = unpackMessage<vector<SM_Variable_Description_t>>(response,offset);
        cout << "Received: " << vars.size() << " vars" << endl;
        for (auto &v : vars)
        {
            rDataNames.push_back(v.name);
            cout << v.name << " " << v.alias << " " << v.unit << " Data:";
            for (auto d : v.data)
            {
                rData.push_back(d);
                //cout << " " << d;
            }
            cout << endl;
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool RemoteHopsanClient::requestSlot(size_t &rControlPort)
{
    sendShortMessage(mRSCSocket, C_ReqSlot);

    zmq::message_t response;
    mRSCSocket.recv(&response);
    size_t offset=0;
    size_t id = getMessageId(response, offset);
    if (id == S_ReqSlot_Reply)
    {
        SM_ReqSlot_Reply_t msg = unpackMessage<SM_ReqSlot_Reply_t>(response, offset);
        rControlPort = msg.port;
        assert(response.size() == offset);
        return true;
    }
    else
    {
        return false;
    }
}

void RemoteHopsanClient::connectToWorker(std::string addres)
{
    mRWCSocket.connect(addres.c_str());
}

bool RemoteHopsanClient::workerConnected()
{
    return mRWCSocket.connected();
}

void RemoteHopsanClient::disconnect()
{
    // Disconnect from Worker
    if (workerConnected())
    {
        sendShortMessage(mRWCSocket, C_Bye);
        readAckNackServerMessage(mRWCSocket); //But we do not care about result
        mRWCSocket.close();
    }

    // Disconnect from Server
    if (serverConnected())
    {
        sendShortMessage(mRSCSocket, C_Bye);
        readAckNackServerMessage(mRSCSocket); //But we do not care about result
        mRSCSocket.close();
    }
}

bool RemoteHopsanClient::requestMessages()
{
    sendShortMessage(mRWCSocket, C_ReqMessages);

    zmq::message_t response;
    mRWCSocket.recv(&response);
    size_t offset=0;
    size_t id = getMessageId(response, offset);
    if (id == S_ReqMessages_Reply)
    {
        vector<SM_HopsanCoreMessage_t> messages = unpackMessage<vector<SM_HopsanCoreMessage_t>>(response, offset);
        cout << "Received: " << messages.size() << " messages from server" << endl;
        for (size_t m=0; m<messages.size(); ++m)
        {
            cout << messages[m].type << " " << messages[m].tag << " " << messages[m].message << endl;
        }
        return true;
    }
    else
    {
        return false;
    }
}


std::string makeAddress(std::string ip, size_t port)
{
    return "tcp://" + ip + ":" + to_string(port);
}
