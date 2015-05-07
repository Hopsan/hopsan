//$Id$

#ifndef REMOTEHOPSANCLIENT_H
#define REMOTEHOPSANCLIENT_H

#include <vector>
#include <string>

#include "ServerStatusMessage.h"
#include "zmq.hpp"

class RemoteHopsanClient
{
public:
    RemoteHopsanClient(zmq::context_t &rContext);
    ~RemoteHopsanClient();
    void setReceiveTimeout(long ms);
    long getReceiveTimeout() const;

    bool connectToServer(std::string zmqaddres);
    bool connectToServer(std::string ip, std::string port);
    bool serverConnected();
    bool requestSlot(size_t &rControlPort);

    bool connectToWorker(std::string zmqaddres);
    bool connectToWorker(std::string ip, std::string port);
    bool workerConnected();

    void disconnect();


    bool sendGetParamMessage(const std::string &rName, std::string &rValue);
    bool sendSetParamMessage(const std::string &rName, const std::string &rValue);
    bool sendModelMessage(const std::string &rModel);
    bool sendSimulateMessage(const int nLogsamples, const int logStartTime, const int simStarttime, const int simSteptime, const int simStoptime);

    bool requestStatus(ServerStatusT &rServerStatus);
    bool requestSimulationResults(std::vector<std::string> *pDataNames, std::vector<double> *pData);
    bool requestMessages();
    bool requestMessages(std::vector<char> &rTypes, std::vector<std::string> &rTags, std::vector<std::string> &rMessages);

    bool requestServerMachines(int nMachines, double maxBenchmarkTime, std::vector<std::string> &rIps, std::vector<std::string> &rPorts);

    std::string getLastErrorMessage() const;

private:
    bool receiveWithTimeout(zmq::socket_t &rSocket, zmq::message_t &rMessage);

    long mReceiveTimeout = 8000; //!< Receive timeout in ms
    std::string mLastErrorMessage;
    std::string mServerAddress;
    std::string mWorkerAddress;
    zmq::socket_t *mpRSCSocket; //!< The Remote Server Control Socket
    zmq::socket_t *mpRWCSocket; //!< The Remote Worker Control Socket

};

#endif // REMOTEHOPSANCLIENT_H
