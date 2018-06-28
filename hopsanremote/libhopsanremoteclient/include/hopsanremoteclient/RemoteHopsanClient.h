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

#ifndef REMOTEHOPSANCLIENT_H
#define REMOTEHOPSANCLIENT_H

#include <vector>
#include <string>
#include <mutex>

#include "hopsanremotecommon/StatusInfoStructs.h"
#include "hopsanremotecommon/DataStructs.h"

// Forward declarations
namespace zmq {
class socket_t;
class context_t;
class message_t;
}

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
    bool requestSlot(int numThreads, int &rControlPort, const std::string userid="");

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
