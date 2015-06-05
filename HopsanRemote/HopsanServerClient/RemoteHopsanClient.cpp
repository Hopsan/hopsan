//$Id$

#include "RemoteHopsanClient.h"

#include "msgpack.hpp"
#include "Messages.h"
#include "MessageUtilities.h"

#include <iostream>
#include <thread>
#include <algorithm>

using namespace std;

// ---------- Help functions start ----------

typedef std::chrono::duration<double> fseconds;
inline double elapsedSecondsSince(chrono::steady_clock::time_point &rSince)
{
    return chrono::duration_cast<fseconds>(chrono::steady_clock::now()-rSince).count();
}

template <typename T>
void sendClientMessage(zmq::socket_t *pSocket, ClientMessageIdEnumT id, const T &rMessage)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    msgpack::pack(out_buffer, rMessage);
    try
    {
        pSocket->send(static_cast<void*>(out_buffer.data()), out_buffer.size());
    }
    catch(zmq::error_t e)
    {
        //Ignore
    }
}


void sendShortClientMessage(zmq::socket_t *pSocket, ClientMessageIdEnumT id)
{
    msgpack::v1::sbuffer out_buffer;
    msgpack::pack(out_buffer, id);
    try
    {
        pSocket->send(static_cast<void*>(out_buffer.data()), out_buffer.size());
    }
    catch(zmq::error_t e)
    {
        //Ignore
    }
}

bool readAckNackServerMessage(zmq::socket_t *pSocket, long timeout, string &rNackReason)
{
    zmq::message_t response;
    if(receiveWithTimeout(*pSocket, timeout, response))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        //cout << "id: " << id << endl;
        if (id == S_Ack)
        {
            return true;
        }
        else if (id == S_NAck)
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

bool readAckNackServerMessage(zmq::socket_t *pSocket, long timeout)
{
    string dummy;
    return readAckNackServerMessage(pSocket, timeout, dummy);
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
        if (mpRSCSocket)
        {
            try
            {
                mpRSCSocket->connect(zmqaddres.c_str());
                mServerAddress = zmqaddres;
                return true;
            }
            catch (zmq::error_t e)
            {
                mLastErrorMessage = e.what();
                mServerAddress.clear();
            }
        }
        return false;
    }
}

bool RemoteHopsanClient::connectToServer(string ip, string port)
{
    return connectToServer(makeZMQAddress(ip,port));
}

bool RemoteHopsanClient::serverConnected() const
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return mpRSCSocket && !mServerAddress.empty();
}


bool RemoteHopsanClient::sendGetParamMessage(const string &rName, string &rValue)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    CM_GetParam_t msg {rName};
    sendClientMessage<CM_GetParam_t>(mpRWCSocket, C_GetParam, msg);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRWCSocket, response, mShortReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == SW_GetParam_Reply)
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

    CM_SetParam_t msg {rName, rValue};
    sendClientMessage<CM_SetParam_t>(mpRWCSocket, C_SetParam, msg);

    string err;
    bool rc = readAckNackServerMessage(mpRWCSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::sendModelMessage(const std::string &rModel)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendClientMessage<std::string>(mpRWCSocket, C_SendingHmf, rModel);
    string err;
    bool rc = readAckNackServerMessage(mpRWCSocket, mShortReceiveTimeout, err);
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

    CM_Simulate_t msg;// {nLogsamples, logStartTime, simStarttime, simSteptime, simStoptime};
    sendClientMessage<CM_Simulate_t>(mpRWCSocket, C_Simulate, msg);
    string err;
    bool rc = readAckNackServerMessage(mpRWCSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::abortSimulation()
{
    // Lock here to prevent problem if some other thread is requesting fore xample status
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpRWCSocket, C_Abort);
    string err;
    bool rc = readAckNackServerMessage(mpRWCSocket, mShortReceiveTimeout, err);
    if (!rc)
    {
        cout << err << endl;
    }
    return rc;
}

bool RemoteHopsanClient::blockingBenchmark(const string &rModel, const int nThreads, double &rSimTime)
{
    bool gotResponse=false;
    size_t workerPort;
    bool rc = requestSlot(nThreads, workerPort);
    if(rc)
    {
        std::string prot,ip,port;
        splitZMQAddress(mServerAddress, prot,ip,port);
        rc = connectToWorker(makeZMQAddress(ip, workerPort));
    }
    else
    {
        cout << mLastErrorMessage << endl;
    }

    if (rc)
    {
        CM_ReqBenchmark_t msg;
        msg.model = rModel;
        sendClientMessage<CM_ReqBenchmark_t>(mpRWCSocket, C_ReqBenchmark, msg);
        string errorMsg;
        rc = readAckNackServerMessage(mpRWCSocket, 5000,  errorMsg);

        if (rc)
        {
            // Block until benchamark is done
            //! @todo what if benchmark freeces, need a timeout here
            double progress; bool isAlive;
            std::thread t(&RemoteHopsanClient::requestWorkerStatusThread, this, &progress, &isAlive);
            t.join();

            // Now request benchmark results
            gotResponse = requestBenchmarkResults(rSimTime);
        }
    }

    disconnectWorker();

    return gotResponse;
}

bool RemoteHopsanClient::requestBenchmarkResults(double &rSimTime)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpRWCSocket, C_ReqBenchmarkResults);
    zmq::message_t reply;
    if (receiveWithTimeout(*mpRWCSocket, reply, mShortReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(reply, offset, parseOK);
        if (id == S_ReqBenchmarkResults_Reply)
        {
            SWM_ReqBenchmarkResults_Reply_t results = unpackMessage<SWM_ReqBenchmarkResults_Reply_t>(reply, offset, parseOK);
            if (parseOK)
            {
                //! @todo right now we bundle all times togheter
                rSimTime = results.inittime+results.simutime+results.finitime;
                return true;
            }
        }
    }
    return false;
}

bool RemoteHopsanClient::requestWorkerStatus(WorkerStatusT &rWorkerStatus)
{
    // Lock here to prevent problem if som other thread is requesting abort
    std::lock_guard<std::mutex> lock(mWorkerMutex); //! @todo should use this mutex in more places (or build it into the send function)

    sendShortClientMessage(mpRWCSocket, C_ReqWorkerStatus);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRWCSocket, response, mShortReceiveTimeout))
    {
        //cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == SW_ReqWorkerStatus_Reply)
        {
            SWM_ReqWorkerStatus_Reply_t status = unpackMessage<SWM_ReqWorkerStatus_Reply_t>(response, offset, parseOK);
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
    sendShortClientMessage(mpRSCSocket, C_ReqServerStatus);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRSCSocket, response, mShortReceiveTimeout))
    {
        //cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == S_ReqServerStatus_Reply)
        {
            SM_ServerStatus_t status = unpackMessage<SM_ServerStatus_t>(response, offset, parseOK);
            if (parseOK)
            {
                rServerStatus = status;
                //cout << "Got status reply" << endl;
                return true;
            }
        }
    }
    return false;
}

bool RemoteHopsanClient::requestSimulationResults(vector<string> *pDataNames, vector<double> *pData)
{
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendClientMessage<string>(mpRWCSocket, C_ReqResults, "*"); // Request all

    zmq::message_t response;
    if (receiveWithTimeout(*mpRWCSocket, response, mLongReceiveTimeout))
    {
        //cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == SW_ReqResults_Reply)
        {
            vector<SM_Variable_Description_t> vars = unpackMessage<vector<SM_Variable_Description_t>>(response,offset, parseOK);
            //cout << "Received: " << vars.size() << " vars" << endl;
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
                //cout << v.name << " " << v.alias << " " << v.unit << " Data:";
                for (auto d : v.data)
                {
                    pData->push_back(d);
                    //cout << " " << d;
                }
                //cout << endl;
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

bool RemoteHopsanClient::requestSlot(int numThreads, size_t &rControlPort)
{
    CM_ReqSlot_t msg {numThreads};
    sendClientMessage<CM_ReqSlot_t>(mpRSCSocket, C_ReqSlot, msg);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRSCSocket, response, mShortReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == S_ReqSlot_Reply)
        {
            SM_ReqSlot_Reply_t msg = unpackMessage<SM_ReqSlot_Reply_t>(response, offset, parseOK);
            if (parseOK)
            {
                rControlPort = msg.port;
                assert(response.size() == offset);
                return true;
            }
            else
            {
                mLastErrorMessage = "Could not parse slot request reply";
            }
        }
        else if (id == S_NAck)
        {
            mLastErrorMessage = unpackMessage<string>(response, offset, parseOK);
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
        if (mpRWCSocket)
        {
            try
            {
                mpRWCSocket->connect(zmqaddres.c_str());
                mWorkerAddress = zmqaddres;
                return true;
            }
            catch (zmq::error_t e)
            {
                mLastErrorMessage =  e.what();
                mWorkerAddress.clear();
            }
        }
        return false;
    }
}

bool RemoteHopsanClient::connectToWorker(string ip, string port)
{
    return connectToWorker(makeZMQAddress(ip,port));
}

bool RemoteHopsanClient::workerConnected() const
{
    // Note we can not use socket.connected() it only checks if underlying c-socket exist
    return mpRWCSocket && !mWorkerAddress.empty();
}

void RemoteHopsanClient::disconnectWorker()
{
    if (workerConnected())
    {
        sendShortClientMessage(mpRWCSocket, C_Bye);
        readAckNackServerMessage(mpRWCSocket, 1000); //But we do not care about result
        try
        {
            mpRWCSocket->disconnect(mWorkerAddress.c_str());
        }
        catch(zmq::error_t e)
        {
            mLastErrorMessage = e.what();
        }
        mWorkerAddress.clear();
    }
}

void RemoteHopsanClient::disconnectServer()
{
    if (serverConnected())
    {
        //! @todo maybe should auto disconnect when we connect to worker
        try
        {
            mpRSCSocket->disconnect(mServerAddress.c_str());
        }
        catch(zmq::error_t e)
        {
            mLastErrorMessage = e.what();
        }
        mServerAddress.clear();
    }
}

void RemoteHopsanClient::disconnect()
{
    // Disconnect from Worker
    disconnectWorker();

    // Disconnect from Server
    disconnectServer();
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

    sendShortClientMessage(mpRWCSocket, C_ReqMessages);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRWCSocket, response, mLongReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == SW_ReqMessages_Reply)
        {
            vector<SM_HopsanCoreMessage_t> messages = unpackMessage<vector<SM_HopsanCoreMessage_t>>(response, offset, parseOK);
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
    std::lock_guard<std::mutex> lock(mWorkerMutex);

    sendShortClientMessage(mpRWCSocket, C_ReqMessages);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRWCSocket, response, mLongReceiveTimeout))
    {
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == SW_ReqMessages_Reply)
        {
            vector<SM_HopsanCoreMessage_t> messages = unpackMessage<vector<SM_HopsanCoreMessage_t>>(response, offset, parseOK);
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

bool RemoteHopsanClient::requestServerMachines(int nMachines, double maxBenchmarkTime, std::vector<string> &rIps, std::vector<string> &rPorts,
                                               std::vector<std::string> &rDescriptions, std::vector<int> &rNumSlots, std::vector<double> &rSpeeds)
{
    CM_ReqServerMachines_t req;
    req.numMachines = nMachines;
    req.maxBenchmarkTime = maxBenchmarkTime;
    req.numThreads = -1;

    sendClientMessage<CM_ReqServerMachines_t>(mpRSCSocket, C_ReqServerMachines, req);

    zmq::message_t response;
    if (receiveWithTimeout(*mpRSCSocket, response, mShortReceiveTimeout))
    {
        cout << "Response size: " << response.size() << endl;
        size_t offset=0;
        bool parseOK;
        size_t id = getMessageId(response, offset, parseOK);
        if (id == S_ReqServerMachines_Reply)
        {
            MSM_ReqServerMachines_Reply_t repl = unpackMessage<MSM_ReqServerMachines_Reply_t>(response,offset,parseOK);
            rIps = repl.ips;
            rPorts = repl.ports;
            rNumSlots = repl.numslots;
            rSpeeds = repl.speeds;
            rDescriptions = repl.descriptions;
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

bool RemoteHopsanClient::receiveWithTimeout(zmq::socket_t &rSocket, zmq::message_t &rMessage, long timeout)
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

void RemoteHopsanClient::deleteSockets()
{
    if (mpRSCSocket)
    {
        try
        {
            delete mpRSCSocket;
        }
        catch(zmq::error_t e)
        {
            mLastErrorMessage = e.what();
        }
        mpRSCSocket = nullptr;
    }
    if (mpRWCSocket)
    {
        try
        {
            delete mpRWCSocket;
        }
        catch(zmq::error_t e)
        {
            mLastErrorMessage = e.what();
        }
        mpRWCSocket = nullptr;
    }
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
        // Ok make an estimate of remaning time based on progressed time
        if (status.simulation_progress > 1e-6)
        {
            // Calulate an estimate of the remaining time
            double  remaining_time = progressedTime / status.simulation_progress - progressedTime;

            // Make sure that we request status within at most mMaxWorkerStatusRequestWaitTime seconds
            // If remaning time lower, then wait estimated remaning time + 100 ms
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


RemoteHopsanClient::RemoteHopsanClient(zmq::context_t &rContext)
{
    int linger_ms = 1000;
    try
    {
        mpRSCSocket = new zmq::socket_t(rContext, ZMQ_REQ);
        mpRWCSocket = new zmq::socket_t(rContext, ZMQ_REQ);
        mpRSCSocket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        mpRWCSocket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
    }
    catch(zmq::error_t e)
    {
        deleteSockets();
        mLastErrorMessage = e.what();
    }
}

RemoteHopsanClient::~RemoteHopsanClient()
{
    deleteSockets();
}

bool RemoteHopsanClient::areSocketsValid() const
{
    return mpRSCSocket && mpRWCSocket;
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
