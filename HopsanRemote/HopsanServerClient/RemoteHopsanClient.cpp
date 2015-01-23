//$Id$

#include "RemoteHopsanClient.h"

#include "msgpack.hpp"
#include "Messages.h"
#include "MessageUtilities.h"

#include <iostream>

using namespace std;

// ---------- Help functions start ----------

template <typename T>
void sendClientMessage(zmq::socket_t &rSocket, ClientMessageIdEnumT id, const T &rMessage)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rMessage);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
}


void sendShortClientMessage(zmq::socket_t &rSocket, ClientMessageIdEnumT id)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    rSocket.send(static_cast<void*>(out_buffer.data()), out_buffer.size());
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

// ---------- Help functions end ----------





void RemoteHopsanClient::connectToServer(std::string zmqaddres)
{
    mRSCSocket.connect(zmqaddres.c_str());
}

void RemoteHopsanClient::connectToServer(string ip, string port)
{
    connectToServer(makeZMQAddress(ip,port));
}

bool RemoteHopsanClient::serverConnected()
{
    return mRSCSocket.connected();
}


bool RemoteHopsanClient::sendGetParamMessage(const string &rName, string &rValue)
{
    CM_GetParam_t msg {rName};
    sendClientMessage<CM_GetParam_t>(mRWCSocket, C_GetParam, msg);

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
    sendClientMessage<CM_SetParam_t>(mRWCSocket, C_SetParam, msg);

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
    sendClientMessage<std::string>(mRWCSocket, C_SendingHmf, rModel);
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
    sendClientMessage<CM_Simulate_t>(mRWCSocket, C_Simulate, msg);
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
    sendClientMessage<string>(mRWCSocket, C_ReqResults, "*"); // Request all
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
    sendShortClientMessage(mRSCSocket, C_ReqSlot);

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

void RemoteHopsanClient::connectToWorker(std::string zmqaddres)
{
    mRWCSocket.connect(zmqaddres.c_str());
}

void RemoteHopsanClient::connectToWorker(string ip, string port)
{
    connectToWorker(makeZMQAddress(ip,port));
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
        sendShortClientMessage(mRWCSocket, C_Bye);
        readAckNackServerMessage(mRWCSocket); //But we do not care about result
        mRWCSocket.close();
    }

    // Disconnect from Server
    if (serverConnected())
    {
        //! @todo maybe should auto disconnect when we connect to worker
        mRSCSocket.close();
    }
}

bool RemoteHopsanClient::requestMessages()
{
    sendShortClientMessage(mRWCSocket, C_ReqMessages);

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
