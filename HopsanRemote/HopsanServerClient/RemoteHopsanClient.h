//$Id$

#ifndef REMOTEHOPSANCLIENT_H
#define REMOTEHOPSANCLIENT_H

#include <vector>
#include <string>

#include "zmq.hpp"

class RemoteHopsanClient
{
public:
    RemoteHopsanClient(zmq::context_t &rContext) : mRSCSocket(rContext, ZMQ_REQ), mRWCSocket(rContext, ZMQ_REQ) {}

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

    bool requestSimulationResults(std::vector<std::string> *pDataNames, std::vector<double> *pData);
    bool requestMessages();
    bool requestMessages(std::vector<char> &rTypes, std::vector<std::string> &rTags, std::vector<std::string> &rMessages);

    std::string getLastErrorMessage() const;

private:
    std::string mLastErrorMessage;
    std::string mServerAddress;
    std::string mWorkerAddress;
    zmq::socket_t mRSCSocket; //!< The Remote Server Control Socket
    zmq::socket_t mRWCSocket; //!< The Remote Worker Control Socket

};

#endif // REMOTEHOPSANCLIENT_H
