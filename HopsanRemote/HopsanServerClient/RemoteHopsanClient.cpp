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

bool readAckNackServerMessage(zmq::socket_t &rSocket, long timeout, string &rNackReason)
{
    zmq::message_t response;
    if(receiveWithTimeout(rSocket, timeout, response))
    {
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
    }
    else
    {
        rNackReason = "Got either timeout or exception in zmq::recv()";
    }
    return false;
}

bool readAckNackServerMessage(zmq::socket_t &rSocket, long timeout)
{
    string dummy;
    return readAckNackServerMessage(rSocket, timeout, dummy);
}

// ---------- Help functions end ----------





bool RemoteHopsanClient::connectToServer(std::string zmqaddres)
{
    if (serverConnected())
    {
        mLastErrorMessage = "You are already connected to a server";
        return false;
    }
    else
    {
        try
        {
            mRSCSocket.connect(zmqaddres.c_str());
            mServerAddress = zmqaddres;
            return true;
        }
        catch (zmq::error_t e)
        {
            mLastErrorMessage =  e.what();
            mServerAddress.clear();
            return false;
        }
    }
}

bool RemoteHopsanClient::connectToServer(string ip, string port)
{
    return connectToServer(makeZMQAddress(ip,port));
}

bool RemoteHopsanClient::serverConnected()
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return !mServerAddress.empty();
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
    bool rc = readAckNackServerMessage(mRWCSocket, mReceiveTimeout, err);
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
    bool rc = readAckNackServerMessage(mRWCSocket, mReceiveTimeout, err);
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
    //! @todo blocking read for now, in the future we need to poll while simulating
    bool rc = readAckNackServerMessage(mRWCSocket, -1, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::requestSimulationResults(vector<string> *pDataNames, vector<double> *pData)
{
    sendClientMessage<string>(mRWCSocket, C_ReqResults, "*"); // Request all

    zmq::message_t response;
    if (receiveWithTimeout(mRWCSocket, response))
    {
        cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        size_t id = getMessageId(response, offset);
        if (id == S_ReqResults_Reply)
        {
            vector<SM_Variable_Description_t> vars = unpackMessage<vector<SM_Variable_Description_t>>(response,offset);
            cout << "Received: " << vars.size() << " vars" << endl;
            size_t nLogSamples;
            if (!vars.empty())
            {
                nLogSamples = vars[0].data.size();
            }

            pDataNames->reserve(vars.size());
            pData->reserve(nLogSamples*vars.size());

            for (auto &v : vars)
            {
                pDataNames->push_back(v.name);
                cout << v.name << " " << v.alias << " " << v.unit << " Data:";
                for (auto d : v.data)
                {
                    pData->push_back(d);
                    //cout << " " << d;
                }
                cout << endl;
            }

            return true;
        }
        else
        {
            mLastErrorMessage = "Got wrong reply";
        }
    }
    return false;
}

bool RemoteHopsanClient::requestSlot(size_t &rControlPort)
{
    sendShortClientMessage(mRSCSocket, C_ReqSlot);

    zmq::message_t response;
    if (receiveWithTimeout(mRSCSocket, response))
    {
        size_t offset=0;
        size_t id = getMessageId(response, offset);
        if (id == S_ReqSlot_Reply)
        {
            SM_ReqSlot_Reply_t msg = unpackMessage<SM_ReqSlot_Reply_t>(response, offset);
            rControlPort = msg.port;
            assert(response.size() == offset);
            return true;
        }
        else if (id == S_NAck)
        {
            mLastErrorMessage = unpackMessage<string>(response, offset);
        }
        else
        {
            mLastErrorMessage = "Got wrong reply type from server";
        }
    }
    return false;
}

bool RemoteHopsanClient::connectToWorker(std::string zmqaddres)
{
    if (workerConnected())
    {
        mLastErrorMessage = "You are already connected to a worker";
        return false;
    }
    else
    {
        try
        {
            mRWCSocket.connect(zmqaddres.c_str());
            mWorkerAddress = zmqaddres;
            return true;
        }
        catch (zmq::error_t e)
        {
            mLastErrorMessage =  e.what();
            mWorkerAddress.clear();
            return false;
        }
    }
}

bool RemoteHopsanClient::connectToWorker(string ip, string port)
{
    return connectToWorker(makeZMQAddress(ip,port));
}

bool RemoteHopsanClient::workerConnected()
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return !mWorkerAddress.empty();
}

void RemoteHopsanClient::disconnect()
{
    // Disconnect from Worker
    if (workerConnected())
    {
        sendShortClientMessage(mRWCSocket, C_Bye);
        readAckNackServerMessage(mRWCSocket, 1000); //But we do not care about result
        mRWCSocket.disconnect(mWorkerAddress.c_str());
        mWorkerAddress.clear();
    }

    // Disconnect from Server
    if (serverConnected())
    {
        //! @todo maybe should auto disconnect when we connect to worker
        mRSCSocket.disconnect(mServerAddress.c_str());
        mServerAddress.clear();
    }
}

bool RemoteHopsanClient::requestMessages()
{
    sendShortClientMessage(mRWCSocket, C_ReqMessages);

    zmq::message_t response;
    if (receiveWithTimeout(mRWCSocket, response))
    {
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
            mLastErrorMessage = "Got wrong reply";
        }
    }
    return false;
}

bool RemoteHopsanClient::requestMessages(std::vector<char> &rTypes, std::vector<string> &rTags, std::vector<string> &rMessages)
{
    sendShortClientMessage(mRWCSocket, C_ReqMessages);

    zmq::message_t response;
    if (receiveWithTimeout(mRWCSocket, response))
    {
        size_t offset=0;
        size_t id = getMessageId(response, offset);
        if (id == S_ReqMessages_Reply)
        {
            vector<SM_HopsanCoreMessage_t> messages = unpackMessage<vector<SM_HopsanCoreMessage_t>>(response, offset);
            //cout << "Received: " << messages.size() << " messages from server" << endl;
            rTypes.resize(messages.size());
            rTags.resize(messages.size());
            rMessages.resize(messages.size());
            for (size_t m=0; m<messages.size(); ++m)
            {
                rTypes[m] =  messages[m].type;
                rTags[m] = messages[m].tag;
                rMessages[m] = messages[m].message;
            }
            return true;
        }
        else
        {
            mLastErrorMessage = "Got wrong reply";
        }
    }
    return false;
}

string RemoteHopsanClient::getLastErrorMessage() const
{
    return mLastErrorMessage;
}

bool RemoteHopsanClient::receiveWithTimeout(zmq::socket_t &rSocket, zmq::message_t &rMessage)
{
    // Create a poll item
    zmq::pollitem_t pollitems[] = {{ rSocket, 0, ZMQ_POLLIN, 0 }};
    try
    {
        // Poll socket for a reply, with timeout
        zmq::poll(&pollitems[0], 1,  mReceiveTimeout);
        // If we have received a message then read message and return true
        if (pollitems[0].revents & ZMQ_POLLIN)
        {
            rSocket.recv(&rMessage);
            return true;
        }
        // Else we reached timeout, return false
        else
        {
            mLastErrorMessage = "Timeout in receive";
        }
    }
    catch(zmq::error_t e)
    {
        mLastErrorMessage = e.what();
    }
    return false;
}


void RemoteHopsanClient::setReceiveTimeout(long ms)
{
    mReceiveTimeout = ms;
}

long RemoteHopsanClient::getReceiveTimeout() const
{
    return mReceiveTimeout;
}
