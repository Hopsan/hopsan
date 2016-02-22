//$Id$

#ifndef REMOTEHOPSANCLIENT_H
#define REMOTEHOPSANCLIENT_H

#include <vector>
#include <string>
#include <mutex>

#include "../include/StatusInfoStructs.h"
#include "../include/DataStructs.h"
#include "zmq.hpp"

class RemoteHopsanClient
{
public:
    RemoteHopsanClient(zmq::context_t &rContext);
    ~RemoteHopsanClient();
    bool areSocketsValid() const;

    void setShortReceiveTimeout(long ms);
    void setLongReceiveTimeout(long ms);
    long getShortReceiveTimeout() const;
    long getLongReceiveTimeout() const;

    void setMaxWorkerStatusRequestWaitTime(double seconds);

    bool connectToAddressServer(std::string address);
    bool addressServerConnected() const;

    bool connectToServer(std::string address);
    bool serverConnected() const;
    bool requestSlot(int numThreads, int &rControlPort);

    bool connectToWorker(int ctrlPort);
    bool workerConnected() const;

    void disconnectAddressServer();
    void disconnectWorker();
    void disconnectServer();
    void disconnect();

    bool sendUserIdentification(const std::string &rUserName, const std::string &rPassword);

    bool blockingSimulation(const int nLogsamples, const int logStartTime, const int simStarttime,
                            const int simSteptime, const int simStoptime, double *pProgress);
    bool blockingBenchmark(const std::string &rModel, const int nThreads, double &rSimTime);

    bool sendGetParamMessage(const std::string &rName, std::string &rValue);
    bool sendSetParamMessage(const std::string &rName, const std::string &rValue);
    bool sendModelMessage(const std::string &rModel);
    bool sendSimulateMessage(const int nLogsamples, const int logStartTime, const int simStarttime,
                             const int simSteptime, const int simStoptime);
    bool executeShellCommand(const std::string &rCommand, std::string output);

    bool blockingRequestFile(const std::string &rRequestName, const std::string &rDestinationFilePath, double *pProgress);
    bool blockingSendFile(const std::string &rAbsFilePath, const std::string &rRelFilePath, double *pProgress);
    bool sendFilePart(const std::string &rRelFilePath, const std::string &rData, bool isLastPart);

    bool abortSimulation();

    bool requestBenchmarkResults(double &rSimTime);
    bool requestWorkerStatus(WorkerStatusT &rWorkerStatus);
    bool requestServerStatus(ServerStatusT &rServerStatus);
    bool requestSimulationResults(std::vector<ResultVariableT> &rResultVariables);
    bool requestMessages();
    bool requestMessages(std::vector<char> &rTypes, std::vector<std::string> &rTags, std::vector<std::string> &rMessages);
    bool requestShellOutput(std::string &rOutput);

    // Address server requests
    bool requestServerMachines(int nMachines, double maxBenchmarkTime, std::vector<ServerMachineInfoT> &rMachines);
    bool requestRelaySlot(const std::string &rBaseRelayIdentity, const int port, std::string &rRelayIdentityFull);
    bool releaseRelaySlot(const std::string &rRelayIdentityFull);

    std::string getLastErrorMessage() const;

private:
    bool receiveWithTimeout(zmq::socket_t &rSocket, zmq::message_t &rMessage, long timeout);
    void deleteSockets();
    void deleteAddressServerSocket();
    void deleteServerSocket();
    void deleteWorkerSocket();
    void requestWorkerStatusThread(double *pProgress, bool *pAlive);
    void setLastError(const std::string &rError);

    double mMaxWorkerStatusRequestWaitTime = 30; //!< The maximum delay between worker status requests in seconds
    long mShortReceiveTimeout = 5000; //!< Receive timeout in ms
    long mLongReceiveTimeout = 30000; //!< Receive timeout in ms
    double mMaxNoProgressTime = 30; //!< The maximum allowed time in seconds with no progress before simulation is assumed frozen
    std::string mLastErrorMessage;
    std::string mAddressServerAddress;
    std::string mServerAddress;
    std::string mWorkerAddress;
    std::string mServerRelayIdentity;
    std::string mWorkerRelayIdentity;
    zmq::socket_t *mpAddressServerSocket = nullptr; //!< The Remote Address Server Control Socket
    zmq::socket_t *mpServerSocket = nullptr; //!< The Remote Server Control Socket
    zmq::socket_t *mpWorkerSocket = nullptr; //!< The Remote Worker Control Socket
    zmq::context_t *mpContext = nullptr;
    std::mutex mWorkerMutex;


};

#endif // REMOTEHOPSANCLIENT_H
