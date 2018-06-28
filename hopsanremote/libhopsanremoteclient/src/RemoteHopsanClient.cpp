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

#include "hopsanremoteclient/RemoteHopsanClient.h"
#include "hopsanremotecommon/Messages.h"
#include "hopsanremotecommon/MessageUtilities.h"
#include "hopsanremotecommon/FileReceiver.hpp"

#include "zmq.hpp"
#include "msgpack.hpp"

#include <iostream>
#include <thread>
#include <algorithm>
#include <fstream>

using namespace std;
const int gLinger_ms = 1000;
#define MAXFILECHUNKSIZE 5000000 //(5 MB)

// ---------- Help functions start ----------

typedef std::chrono::duration<double> fseconds;
inline double elapsedSecondsSince(chrono::steady_clock::time_point &rSince)
{
    return chrono::duration_cast<fseconds>(chrono::steady_clock::now()-rSince).count();
}

template <typename T>
void sendClientMessage(zmq::socket_t *pSocket, MessageIdsEnumT id, const T &rMessage)
{
    try
    {
        sendMessage(*pSocket, id, rMessage);
        //pSocket->send(static_cast<void*>(out_buffer.data()), out_buffer.size());
    }
    catch(zmq::error_t e)
    {
        //Ignore
    }
}


void sendShortClientMessage(zmq::socket_t *pSocket, MessageIdsEnumT id)
{
    try
    {
        sendShortMessage(*pSocket, id);
    }
    catch(zmq::error_t e)
    {
        //Ignore
    }
}

bool receiveAckNackMessage(zmq::socket_t *pSocket, long timeout, string &rNackReason)
{
    zmq::message_t response;
    if(receiveWithTimeout(*pSocket, timeout, response))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        //cout << "id: " << id << endl;
        if (id == Ack)
        {
            return true;
        }
        else if (id == NotAck)
        {
            rNackReason = unpackMessage<std::string>(response, offset, parseOK);
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

bool receiveAckNackMessage(zmq::socket_t *pSocket, long timeout)
{
    string dummy;
    return receiveAckNackMessage(pSocket, timeout, dummy);
}

// ---------- Help functions end ----------

bool RemoteHopsanClient::connectToServer(std::string address)
{
    if (serverConnected())
    {
        setLastError("You are already connected to a server");
        return false;
    }
    else
    {
        try
        {
            mpServerSocket = new zmq::socket_t(*mpContext, ZMQ_REQ);
            mpServerSocket->setsockopt(ZMQ_LINGER, &gLinger_ms, sizeof(int));

            std::string ip,port,fullrelayid,baserelayid,subrelayid;
            splitaddressandrelayid(address,ip,port,fullrelayid,baserelayid,subrelayid);

            if(!baserelayid.empty() && subrelayid.empty() && addressServerConnected())
            {
                requestRelaySlot(baserelayid,-1,fullrelayid);
            }

            mServerRelayIdentity = fullrelayid;
            mServerAddress = makeZMQAddress(ip,port);

            if (!mServerRelayIdentity.empty())
            {
                mpServerSocket->setsockopt( ZMQ_IDENTITY, (void*)mServerRelayIdentity.data(), mServerRelayIdentity.size());
                //cout << " Set server relay socket identity: " << mServerRelayIdentity << endl;
            }
            mpServerSocket->connect(mServerAddress.c_str());
            return true;
        }
        catch (zmq::error_t e)
        {
            setLastError(e.what());
            deleteServerSocket();
        }
        return false;
    }
}

bool RemoteHopsanClient::serverConnected() const
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return mpServerSocket && !mServerAddress.empty();
}


bool RemoteHopsanClient::sendGetParamMessage(const string &rName, string &rValue)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    ReqmsgRequestParameter msg {rName};
    sendClientMessage(mpWorkerSocket, RequestParameter, msg);

    zmq::message_t response;
    if (receiveWithTimeout(*mpWorkerSocket, response, mShortReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyParameter)
        {
            rValue = unpackMessage<string>(response, offset, parseOK);
            assert(response.size() == offset);
            return true;
        }
        else
        {
            rValue.clear();
            return false;
        }
    }
    return false;
}

bool RemoteHopsanClient::sendSetParamMessage(const string &rName, const string &rValue)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    CmdmsgSetParameter msg {rName, rValue};
    sendClientMessage(mpWorkerSocket, SetParameter, msg);

    string err;
    bool rc = receiveAckNackMessage(mpWorkerSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::sendModelMessage(const std::string &rModel)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendClientMessage<std::string>(mpWorkerSocket, SetModel, rModel);
    string err;
    bool rc = receiveAckNackMessage(mpWorkerSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::sendSimulateMessage(const int nLogsamples, const int logStartTime,
                         const int simStarttime, const int simSteptime, const int simStoptime)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    CmdmsgSimulate msg;// {nLogsamples, logStartTime, simStarttime, simSteptime, simStoptime};
    sendClientMessage(mpWorkerSocket, Simulate, msg);
    string err;
    bool rc = receiveAckNackMessage(mpWorkerSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::executeShellCommand(const string &rCommand, std::string output)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);
    sendClientMessage(mpWorkerSocket, ExecuteInShell, rCommand);
    string err;
    bool rc = receiveAckNackMessage(mpWorkerSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        output = err;
    }
    return rc;
}

bool RemoteHopsanClient::blockingRequestFile(const string &rRequestName, const string &rDestinationFilePath, double *pProgress)
{
    *pProgress=0;
    bool isOK = true;
    std::lock_guard<std::mutex> lock(mWorkerMutex);
    FileReceiver fr;

    bool isDone=false;
    int fileoffset = 0;

    while (!isDone)
    {
        CmdmsgRequestFile requestmsg {rRequestName, fileoffset};
        sendClientMessage(mpWorkerSocket, RequestFile, requestmsg);

        zmq::message_t reply;
        string err;

        bool unpackOK;
        size_t zmqoffset=0;
        bool rc = receiveWithTimeout(*mpWorkerSocket, reply, mLongReceiveTimeout);
        if (rc)
        {
            int msgid = getMessageId(reply, zmqoffset, unpackOK);
            if (msgid==SendFile && unpackOK)
            {
                CmdmsgSendFile replymsg = unpackMessage<CmdmsgSendFile>(reply, zmqoffset, unpackOK);
                if (unpackOK)
                {
                    bool addOK = fr.addFilePart(replymsg, err);
                    if (!addOK)
                    {
                        cout << "Failed to write file part: " << rRequestName << " to: " << rDestinationFilePath << endl;
                        isOK = false;
                    }
                    fileoffset+=replymsg.data.size();
                    if (replymsg.islastpart)
                    {
                        isDone=true;
                    }
                }
                else
                {
                    cout << "Failed to unpack file part: " << rRequestName << endl;
                    isOK = false;
                }
            }
            else if (msgid == NotAck && unpackOK)
            {
                string err = unpackMessage<std::string>(reply, zmqoffset, unpackOK);
                cout << "Failed in file request: " << err << endl;
                isOK=false;
            }
        }
        else
        {
            cout << "Failed to receive file (request timeout): " << rRequestName << endl;
            isOK = false;
        }

        if (!isOK)
        {
            break;
        }
    }

    return isOK;
}

//! @brief Send a file (blocking until the entire file is transferred)
//! @param[in] rAbsFilePath The absolute file path, (what file to read from on the local machine)
//! @param[in] rRelFilePath The file path "relative to the model", (the file path entered in the model parameter value), this is also the file identifier
//! @param[out] pProgress The transfer progress 0..1
bool RemoteHopsanClient::blockingSendFile(const string &rAbsFilePath, const string &rRelFilePath, double *pProgress)
{
    *pProgress=0;
    std::ifstream in(rAbsFilePath, std::ifstream::ate | std::ifstream::binary);
    if (in.is_open())
    {
        std::ifstream::pos_type filesize = in.tellg();
        in.seekg(0); //Rewind file ptr
        std::vector<char>  buffer(MAXFILECHUNKSIZE);

        std::ifstream::pos_type readBytesNow, remaningBytes=filesize;
        while( !in.eof() && (remaningBytes != 0) )
        {
            readBytesNow = std::min(std::ifstream::pos_type(MAXFILECHUNKSIZE), remaningBytes);
            in.read(buffer.data(), readBytesNow);
            std::string data(buffer.data(), readBytesNow);
            bool rc = sendFilePart(rRelFilePath, data, (readBytesNow < MAXFILECHUNKSIZE));
            if (!rc)
            {
                cout << "Failed to send file part! " << endl;
                in.close();
                return false;
            }
            remaningBytes -= readBytesNow;
            *pProgress = double(filesize-remaningBytes)/double(filesize);
            //cout << "fileSize: " << filesize << " bytesNow: "<< readBytesNow << " remaningBytes: " << remaningBytes << " isDone: " << (readBytesNow < MAXFILECHUNKSIZE) << " Progress: " << *pProgress << endl;
        }
    }
    else
    {
        cout << "Could not open file: " << rAbsFilePath << endl;
        return false;
    }
    in.close();
    return true;
}

//! @brief Send a part of a file (blocking with timeout)
//! @param[in] rRelFilePath The filepath "relative to the model", (the file path entered in the model parameter value), this is also the file identifier
//! @param[in] rData The data chunk buffer (as a string)
//! @param[in] isLastPart Signal that this is the last part of the file
bool RemoteHopsanClient::sendFilePart(const string &rRelFilePath, const string &rData, bool isLastPart)
{
   std::lock_guard<std::mutex> lock(mWorkerMutex);
   CmdmsgSendFile msg {rRelFilePath, rData, isLastPart};
   sendClientMessage(mpWorkerSocket, SendFile, msg);
   string err;
   bool rc = receiveAckNackMessage(mpWorkerSocket, mLongReceiveTimeout, err);
   if (!rc)
   {
       cout << err << endl;
   }
   return rc;
}

bool RemoteHopsanClient::abortSimulation()
{
    // Lock here to prevent problem if some other thread is requesting for example status
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpWorkerSocket, Abort);
    string err;
    bool rc = receiveAckNackMessage(mpWorkerSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::blockingBenchmark(const string &rModel, const int nThreads, double &rSimTime)
{
    bool gotResponse=false;
    int ctrlPort;
    bool rc = requestSlot(nThreads, ctrlPort);
    if(rc)
    {
        rc = connectToWorker(ctrlPort);
    }
    else
    {
        cout << mLastErrorMessage << endl;
    }

    if (rc)
    {
        CmdmsgBenchmark msg;
        msg.model = rModel;
        sendClientMessage(mpWorkerSocket, Benchmark, msg);
        string errorMsg;
        rc = receiveAckNackMessage(mpWorkerSocket, 5000,  errorMsg);

        if (rc)
        {
            // Block until benchmark is done
            //! @todo what if benchmark freezes, need a timeout here
            double progress; bool isAlive;
            std::thread t(&RemoteHopsanClient::requestWorkerStatusThread, this, &progress, &isAlive);
            t.join();

            // Now request benchmark results
            gotResponse = requestBenchmarkResults(rSimTime);
        }
        else
        {
            cout << errorMsg << endl;
        }
    }

    disconnectWorker();

    return gotResponse;
}

bool RemoteHopsanClient::requestBenchmarkResults(double &rSimTime)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpWorkerSocket, RequestBenchmarkResults);
    zmq::message_t reply;
    if (receiveWithTimeout(*mpWorkerSocket, reply, mShortReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(reply, offset, parseOK);
        if (id == ReplyBenchmarkResults)
        {
            ReplymsgReplyBenchmarkResults results = unpackMessage<ReplymsgReplyBenchmarkResults>(reply, offset, parseOK);
            if (parseOK)
            {
                //! @todo right now we bundle all times together
                rSimTime = results.inittime+results.simutime+results.finitime;
                return true;
            }
        }
    }
    return false;
}

bool RemoteHopsanClient::requestWorkerStatus(WorkerStatusT &rWorkerStatus)
{
    // Lock here to prevent problem if some other thread is requesting abort
    std::lock_guard<std::mutex> lock(mWorkerMutex); //! @todo should use this mutex in more places (or build it into the send function)

    sendShortClientMessage(mpWorkerSocket, RequestWorkerStatus);

    zmq::message_t response;
    if (receiveWithTimeout(*mpWorkerSocket, response, mShortReceiveTimeout))
    {
        //cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyWorkerStatus)
        {
            ReplymsgReplyWorkerStatus status = unpackMessage<ReplymsgReplyWorkerStatus>(response, offset, parseOK);
            if (parseOK)
            {
                rWorkerStatus = status;
                return true;
            }
        }
    }
    return false;
}

bool RemoteHopsanClient::requestServerStatus(ServerStatusT &rServerStatus)
{
    sendShortClientMessage(mpServerSocket, RequestServerStatus);

    zmq::message_t response;
    if (receiveWithTimeout(*mpServerSocket, response, mShortReceiveTimeout))
    {
        //cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyServerStatus)
        {
            ReplymsgReplyServerStatus status = unpackMessage<ReplymsgReplyServerStatus>(response, offset, parseOK);
            if (parseOK)
            {
                rServerStatus = status;
                //cout << "Got status reply" << endl;
                return true;
            }
            cout << "Parse error" << endl;
        }
        else if (id == NotAck)
        {
            std::string err = unpackMessage<std::string>(response, offset, parseOK);
            cout << "Received NotAck: " << err << endl;
        }
    }
    return false;
}

bool RemoteHopsanClient::requestSimulationResults(std::vector<ResultVariableT> &rResultVariables)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendClientMessage<string>(mpWorkerSocket, RequestResults, "*"); // Request all

    zmq::message_t response;
    if (receiveWithTimeout(*mpWorkerSocket, response, mLongReceiveTimeout))
    {
        //cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyResults)
        {
            std::vector<ReplymsgResultsVariable> repls = unpackMessage<vector<ReplymsgResultsVariable>>(response,offset, parseOK);
            rResultVariables.clear();
            rResultVariables.reserve(repls.size());
            for (ReplymsgResultsVariable &repl : repls)
            {
                rResultVariables.push_back(repl);
            }
            return parseOK;
        }
        else
        {
            setLastError("Got wrong reply");
        }
    }
    return false;
}

bool RemoteHopsanClient::requestSlot(int numThreads, int &rControlPort, const std::string userid)
{
    ReqmsgReqServerSlots msg {numThreads, userid};
    sendClientMessage<ReqmsgReqServerSlots>(mpServerSocket, RequestServerSlots, msg);

    zmq::message_t response;
    if (receiveWithTimeout(*mpServerSocket, response, mShortReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyServerSlots)
        {
            ReplymsgReplyServerSlots msg = unpackMessage<ReplymsgReplyServerSlots>(response, offset, parseOK);
            if (parseOK)
            {
                rControlPort = msg.portoffset;
                assert(response.size() == offset);
                return true;
            }
            else
            {
                setLastError("Could not parse slot request reply");
            }
        }
        else if (id == NotAck)
        {
            setLastError(unpackMessage<string>(response, offset, parseOK));
        }
        else
        {
            setLastError("Got wrong reply type from server");
        }
    }
    return false;
}

bool RemoteHopsanClient::connectToWorker(int workerPort)
{
    if (workerConnected())
    {
        setLastError("You are already connected to a worker");
        return false;
    }
    else
    {
        try
        {
            mpWorkerSocket = new zmq::socket_t(*mpContext, ZMQ_REQ);
            mpWorkerSocket->setsockopt(ZMQ_LINGER, &gLinger_ms, sizeof(int));

            std::string proto, srvip, srvport;
            splitZMQAddress(mServerAddress, proto, srvip, srvport);

            if (mServerRelayIdentity.empty())
            {
                mWorkerAddress = makeZMQAddress(srvip, workerPort);
            }
            else
            {
                vector<string> fields = splitstring(mServerRelayIdentity, ".");
                string fullRelayIdentity;
                requestRelaySlot(fields.front(), workerPort, fullRelayIdentity);

                mWorkerAddress = makeZMQAddress(srvip, srvport);
                mWorkerRelayIdentity = fullRelayIdentity;
                if (!mWorkerRelayIdentity.empty())
                {
                    mpWorkerSocket->setsockopt( ZMQ_IDENTITY, (void*)mWorkerRelayIdentity.data(), mWorkerRelayIdentity.size());
                }
            }
            mpWorkerSocket->connect(mWorkerAddress.c_str());
            return true;
        }
        catch (zmq::error_t e)
        {
            setLastError( e.what() );
            deleteWorkerSocket();
        }
    }
    return false;
}


bool RemoteHopsanClient::workerConnected() const
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return mpWorkerSocket && !mWorkerAddress.empty();
}

void RemoteHopsanClient::disconnectAddressServer()
{
    if (addressServerConnected())
    {
        sendShortClientMessage(mpAddressServerSocket, ClientClosing);
        receiveAckNackMessage(mpAddressServerSocket, 1000); //But we do not care about result
        try
        {
            mpAddressServerSocket->disconnect(mAddressServerAddress.c_str());
            mpAddressServerSocket->close();
        }
        catch(zmq::error_t e)
        {
            setLastError(e.what());
        }
        deleteAddressServerSocket();
    }
}

void RemoteHopsanClient::disconnectWorker()
{
    if (workerConnected())
    {
        sendShortClientMessage(mpWorkerSocket, ClientClosing);
        receiveAckNackMessage(mpWorkerSocket, 1000); //But we do not care about result
        try
        {
            mpWorkerSocket->disconnect(mWorkerAddress.c_str());
            mpWorkerSocket->close();
            if (!mWorkerRelayIdentity.empty())
            {
                releaseRelaySlot(mWorkerRelayIdentity);
            }
        }
        catch(zmq::error_t e)
        {
            setLastError(e.what());
        }
        mWorkerAddress.clear();
        mWorkerRelayIdentity.clear();
    }
    deleteWorkerSocket();
}

void RemoteHopsanClient::disconnectServer()
{
    if (serverConnected())
    {
        sendShortClientMessage(mpServerSocket, ClientClosing);
        receiveAckNackMessage(mpServerSocket, 1000); //But we do not care about result
        //! @todo maybe should auto disconnect when we connect to worker
        try
        {
            mpServerSocket->disconnect(mServerAddress.c_str());
            mpServerSocket->close();
            if (!mServerRelayIdentity.empty())
            {
                releaseRelaySlot(mServerRelayIdentity);
            }
        }
        catch(zmq::error_t e)
        {
            setLastError(e.what());
        }
        mServerAddress.clear();
        mServerRelayIdentity.clear();
    }
    deleteServerSocket();
}

void RemoteHopsanClient::disconnect()
{
    // Disconnect from Worker
    disconnectWorker();

    // Disconnect from Server
    disconnectServer();

    // Disconnect from address server
    disconnectAddressServer();
}

bool RemoteHopsanClient::sendUserIdentification(const string &rUserName, const string &rPassword)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);
    CmdmsgIdentifyUser msg = {rUserName, rPassword};
    sendClientMessage(mpWorkerSocket, IdentifyUser, msg);
    std::string nackmsg;
    bool rc = receiveAckNackMessage(mpWorkerSocket, mShortReceiveTimeout, nackmsg);
    if (!rc)
    {
        setLastError(nackmsg);
    }
    return rc;
}

bool RemoteHopsanClient::blockingSimulation(const int nLogsamples, const int logStartTime, const int simStarttime,
                                            const int simSteptime, const int simStoptime, double *pProgress)
{
    bool initOK = sendSimulateMessage(nLogsamples, logStartTime, simStarttime, simSteptime, simStoptime);
    if (initOK)
    {
        bool isAlive;
        std::thread t(&RemoteHopsanClient::requestWorkerStatusThread, this, pProgress, &isAlive);
        t.join();
        WorkerStatusT status;
        requestWorkerStatus(status);
        return (status.model_loaded && status.simualtion_success);
    }
    return initOK;
}

bool RemoteHopsanClient::requestMessages()
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpWorkerSocket, RequestMessages);

    zmq::message_t response;
    if (receiveWithTimeout(*mpWorkerSocket, response, mLongReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyMessages)
        {
            vector<ReplymsgReplyMessage> messages = unpackMessage<vector<ReplymsgReplyMessage>>(response, offset, parseOK);
            cout << "Received: " << messages.size() << " messages from server" << endl;
            for (size_t m=0; m<messages.size(); ++m)
            {
                cout << messages[m].type << " " << messages[m].tag << " " << messages[m].message << endl;
            }
            return true;
        }
        else
        {
            setLastError("Got wrong reply");
        }
    }
    return false;
}

bool RemoteHopsanClient::requestMessages(std::vector<char> &rTypes, std::vector<string> &rTags, std::vector<string> &rMessages)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpWorkerSocket, RequestMessages);

    zmq::message_t response;
    if (receiveWithTimeout(*mpWorkerSocket, response, mLongReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyMessages)
        {
            vector<ReplymsgReplyMessage> messages = unpackMessage<vector<ReplymsgReplyMessage>>(response, offset, parseOK);
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
            setLastError("Got wrong reply");
        }
    }
    return false;
}

bool RemoteHopsanClient::requestShellOutput(string &rOutput)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpWorkerSocket, RequestShellOutput);

    zmq::message_t response;
    if (receiveWithTimeout(*mpWorkerSocket, response, mLongReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == ReplyShellOutput)
        {
            rOutput = unpackMessage<string>(response, offset, parseOK);
            return parseOK;
        }
        else
        {
            setLastError("Got wrong reply");
        }
    }
    return false;
}

bool RemoteHopsanClient::requestServerMachines(int nMachines, double maxBenchmarkTime, std::vector<ServerMachineInfoT> &rMachines)
{
    if (addressServerConnected())
    {
        ReqmsgRequestServerMachines req;
        req.numMachines = nMachines;
        req.maxBenchmarkTime = maxBenchmarkTime;
        req.numThreads = -1;

        sendClientMessage(mpAddressServerSocket, RequestServerMachines, req);

        zmq::message_t response;
        if (receiveWithTimeout(*mpAddressServerSocket, response, mShortReceiveTimeout))
        {
            //cout << "Response size: " << response.size() << endl;
            size_t offset=0;
            bool parseOK;
            size_t id = getMessageId(response, offset, parseOK);
            if (id == ReplyServerMachines)
            {
                std::vector<ReplymsgReplyServerMachine> repl = unpackMessage<std::vector<ReplymsgReplyServerMachine>>(response,offset,parseOK);
                rMachines.reserve(repl.size());
                for (auto &rMachine : repl)
                {
                    rMachines.push_back(rMachine);
                }
                return true;
            }
            else
            {
                setLastError("Got wrong reply");
            }
        }
    }
    return false;
}

bool RemoteHopsanClient::requestRelaySlot(const std::string &rBaseRelayIdentity, const int port, std::string &rRelayIdentityFull)
{
    if (addressServerConnected())
    {
        std::vector<std::string> fields = splitstring(rBaseRelayIdentity, "."); // Split in case socket id included (we dont want that)
        ReqmsgRelaySlot msg {fields.front(), port};
        sendClientMessage(mpAddressServerSocket, RequestRelaySlot, msg);

        zmq::message_t response;
        if (receiveWithTimeout(*mpAddressServerSocket, response, mShortReceiveTimeout))
        {
            size_t offset=0;
            bool parseOK;
            size_t id = getMessageId(response, offset, parseOK);
            if (id == ReplyRelaySlot)
            {
                rRelayIdentityFull = unpackMessage<std::string>(response,offset,parseOK);
                return true;
            }
            else if (id == NotAck)
            {
                setLastError(unpackMessage<std::string>(response, offset, parseOK));
            }
            else
            {
                setLastError("Received wrong reply");
            }
        }
    }
    rRelayIdentityFull.clear();
    return false;
}

bool RemoteHopsanClient::releaseRelaySlot(const string &rRelayIdentityFull)
{
    if (addressServerConnected())
    {
        sendClientMessage(mpAddressServerSocket, ReleaseRelaySlot, rRelayIdentityFull);
        string err;
        bool rc = receiveAckNackMessage(mpAddressServerSocket, mShortReceiveTimeout, err);
        if (!rc)
        {
            setLastError(err);
        }
        return rc;
    }
    return false;
}

string RemoteHopsanClient::getLastErrorMessage() const
{
    return mLastErrorMessage;
}

bool RemoteHopsanClient::receiveWithTimeout(zmq::socket_t &rSocket, zmq::message_t &rMessage, long timeout)
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
        else
        {
            setLastError("Timeout in receive");
        }
    }
    catch(zmq::error_t e)
    {
        setLastError(e.what());
    }
    return false;
}

void RemoteHopsanClient::deleteSockets()
{
    deleteAddressServerSocket();
    deleteServerSocket();
    deleteWorkerSocket();
}

void RemoteHopsanClient::deleteAddressServerSocket()
{
    if (mpAddressServerSocket)
    {
        try
        {
            delete mpAddressServerSocket;
        }
        catch(zmq::error_t e)
        {
            setLastError(e.what());
        }
        mpAddressServerSocket = nullptr;
    }
    mAddressServerAddress.clear();
}

void RemoteHopsanClient::deleteServerSocket()
{
    if (mpServerSocket)
    {
        try
        {
            delete mpServerSocket;
        }
        catch(zmq::error_t e)
        {
            setLastError(e.what());
        }
        mpServerSocket = nullptr;
    }
    mServerRelayIdentity.clear();
    mServerAddress.clear();
}

void RemoteHopsanClient::deleteWorkerSocket()
{
    if (mpWorkerSocket)
    {
        try
        {
            delete mpWorkerSocket;
        }
        catch(zmq::error_t e)
        {
            setLastError(e.what());
        }
        mpWorkerSocket = nullptr;
    }
    mWorkerRelayIdentity.clear();
    mWorkerAddress.clear();
}

void RemoteHopsanClient::requestWorkerStatusThread(double *pProgress, bool *pAlive)
{
    const int defaultSleepMs = 200;
    double progressedTime = 0;
    double lastProgress=0;
    bool firstNoProgress = true;

    // Assume server alive
    // Note! This one should be set true from the outside since it may take some time to launch this thread
    *pAlive = true;

    WorkerStatusT status;
    chrono::milliseconds msd{defaultSleepMs};
    chrono::milliseconds ms=msd;
    chrono::steady_clock::time_point startT = chrono::steady_clock::now();
    chrono::steady_clock::time_point lastNoProgressTime = chrono::steady_clock::now();

    bool rc = requestWorkerStatus(status);
    *pProgress = status.simulation_progress;
    *pAlive = rc;
    while (!status.simulation_finished && rc)
    {
        std::this_thread::sleep_for(ms);
        progressedTime = elapsedSecondsSince(startT);
        rc = requestWorkerStatus(status);
        *pProgress = status.simulation_progress;
        // OK make an estimate of remaining time based on progressed time
        if (status.simulation_progress > 1e-6)
        {
            // Calculate an estimate of the remaining time
            double  remaining_time = progressedTime / status.simulation_progress - progressedTime;

            // Make sure that we request status within at most mMaxWorkerStatusRequestWaitTime seconds
            // If remaining time lower, then wait estimated remaining time + 100 ms
            double sleep_time = min( remaining_time+0.1, mMaxWorkerStatusRequestWaitTime );
            if (sleep_time > 0 )
            {
                std::chrono::milliseconds mss{int(sleep_time*1000.0)};
                ms = mss;
            }
            else
            {
                ms = msd;
            }

            if (status.simulation_progress == lastProgress)
            {
                if (firstNoProgress)
                {
                    lastNoProgressTime = chrono::steady_clock::now();
                    firstNoProgress = false;
                }
                else if (elapsedSecondsSince(lastNoProgressTime) > mMaxNoProgressTime)
                {
                    *pAlive = false;
                    break;
                }
            }
            else
            {
                firstNoProgress = true;
            }

            lastProgress = status.simulation_progress;
        }
    }
}

void RemoteHopsanClient::setLastError(const string &rError)
{
    mLastErrorMessage = rError;
    cout << "RemoteHopsanClient Error: " << rError << endl;
}


RemoteHopsanClient::RemoteHopsanClient(zmq::context_t &rContext)
{
    mpContext = &rContext;
}

RemoteHopsanClient::~RemoteHopsanClient()
{
    disconnect();
    deleteSockets();
}

bool RemoteHopsanClient::areSocketsValid() const
{
    return mpServerSocket && mpWorkerSocket;
}

void RemoteHopsanClient::setShortReceiveTimeout(long ms)
{
    mShortReceiveTimeout = ms;
}

void RemoteHopsanClient::setLongReceiveTimeout(long ms)
{
    mLongReceiveTimeout = ms;
}

long RemoteHopsanClient::getShortReceiveTimeout() const
{
    return mShortReceiveTimeout;
}

long RemoteHopsanClient::getLongReceiveTimeout() const
{
    return mLongReceiveTimeout;
}

void RemoteHopsanClient::setMaxWorkerStatusRequestWaitTime(double seconds)
{
    mMaxWorkerStatusRequestWaitTime = seconds;
}

bool RemoteHopsanClient::connectToAddressServer(string address)
{
    if (addressServerConnected())
    {
        setLastError("You are already connected to an address server");
        return false;
    }
    else
    {
        if (!mpAddressServerSocket)
        {
            int linger_ms = 1000;
            try
            {
                mpAddressServerSocket = new zmq::socket_t(*mpContext, ZMQ_REQ);
                mpAddressServerSocket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
            }
            catch(zmq::error_t e)
            {
                setLastError(e.what());
                deleteAddressServerSocket();
            }
        }

        if (mpAddressServerSocket)
        {
            string ip,port,relayid;
            splitaddress(address, ip,port,relayid);
            try
            {
                mAddressServerAddress = makeZMQAddress(ip,port);
                mpAddressServerSocket->connect(mAddressServerAddress.c_str());
                return true;
            }
            catch (zmq::error_t e)
            {
                setLastError(e.what());
                disconnectAddressServer();
            }
        }
        return false;
    }
}

bool RemoteHopsanClient::addressServerConnected() const
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return mpAddressServerSocket && !mAddressServerAddress.empty();
}
