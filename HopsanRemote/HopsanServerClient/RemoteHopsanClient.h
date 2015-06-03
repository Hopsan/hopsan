//$Id$

#ifndef REMOTEHOPSANCLIENT_H
#define REMOTEHOPSANCLIENT_H

#include <vector>
#include <string>
#include <mutex>

#include "ServerStatusMessage.h"
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

    bool connectToServer(std::string zmqaddres);
    bool connectToServer(std::string ip, std::string port);
    bool serverConnected() const;
    bool requestSlot(int numThreads, size_t &rControlPort);

    bool connectToWorker(std::string zmqaddres);
    bool connectToWorker(std::string ip, std::string port);
    bool workerConnected() const;

    void disconnectWorker();
    void disconnectServer();
    void disconnect();

    bool blockingSimulation(const int nLogsamples, const int logStartTime, const int simStarttime,
                            const int simSteptime, const int simStoptime, double *pProgress);
    bool blockingBenchmark(const std::string &rModel, const int nThreads, double &rSimTime);

    bool sendGetParamMessage(const std::string &rName, std::string &rValue);
    bool sendSetParamMessage(const std::string &rName, const std::string &rValue);
    bool sendModelMessage(const std::string &rModel);
    bool sendSimulateMessage(const int nLogsamples, const int logStartTime, const int simStarttime,
                             const int simSteptime, const int simStoptime);

    bool abortSimulation();

    bool requestBenchmarkResults(double &rSimTime);
    bool requestWorkerStatus(WorkerStatusT &rWorkerStatus);
    bool requestServerStatus(ServerStatusT &rServerStatus);
    bool requestSimulationResults(std::vector<std::string> *pDataNames, std::vector<double> *pData);
    bool requestMessages();
    bool requestMessages(std::vector<char> &rTypes, std::vector<std::string> &rTags, std::vector<std::string> &rMessages);

    bool requestServerMachines(int nMachines, double maxBenchmarkTime, std::vector<std::string> &rIps, std::vector<std::string> &rPorts,
                               std::vector<std::string> &rDescriptions, std::vector<int> &rNumSlots, std::vector<double> &rSpeeds);

    std::string getLastErrorMessage() const;

private:
    bool receiveWithTimeout(zmq::socket_t &rSocket, zmq::message_t &rMessage, long timeout);
    void deleteSockets();
    void requestWorkerStatusThread(double *pProgress);

    double mMaxWorkerStatusRequestWaitTime = 30; //!< The maximum delay between worker status requests in seconds
    long mShortReceiveTimeout = 5000; //!< Receive timeout in ms
    long mLongReceiveTimeout = 30000; //!< Receive timeout in ms
    std::string mLastErrorMessage;
    std::string mServerAddress;
    std::string mWorkerAddress;
    zmq::socket_t *mpRSCSocket = nullptr; //!< The Remote Server Control Socket
    zmq::socket_t *mpRWCSocket = nullptr; //!< The Remote Worker Control Socket
    std::mutex mWorkerMutex;

};

#endif // REMOTEHOPSANCLIENT_H
