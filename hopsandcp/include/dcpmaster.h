#ifndef DCPMASTER_H
#define DCPMASTER_H

#include "hopsandcp_win32dll.h"

#include "dcp/model/constant/DcpState.hpp"
#include "dcp/model/constant/DcpError.hpp"
#include "dcp/log/OstreamLog.hpp"

#include <vector>
#include <map>

using namespace std;

class DcpManagerMaster;
class UdpDriver;
class SlaveDescription_t;

struct DcpConnection
{
    size_t fromSlave, fromVr;
    std::vector<size_t> toSlaves, toVrs;
};

HOPSANDCP_DLLAPI class DcpMaster
{
public:
    DcpMaster(const string host, u_short port);
    ~DcpMaster();

    void addSlave(std::string &filepath);
    void addConnection(size_t fromId, size_t fromVr, std::vector<size_t> toIds, std::vector<size_t> toVrs);

    void start();
private:
    void initialize();
    void configuration();
    void configure();
    void run(DcpState currentState, uint8_t sender);
    void doStep();
    void stop();
    void deregister();
    void sendOutputs(DcpState currentState, uint8_t sender);
    void receiveAck(uint8_t sender, uint16_t);
    void receiveNAck(uint8_t sender, uint16_t pduSeqId, DcpError errorCode);
    void dataReceived(uint16_t dataId, size_t length, uint8_t payload[]);
    void receiveStateChangedNotification(uint8_t sender, DcpState state);

    std::vector<DcpConnection> connections;

    uint8_t maxInitRuns = 0;
    uint8_t intializationRuns = 1;

    std::map<uint8_t, DcpState> curState;

    UdpDriver *driver;

    OstreamLog stdLog;

    DcpManagerMaster *manager;

    uint64_t nSteps=0;
    uint64_t secondsToSimulate = 10;
    std::map<uint8_t, uint8_t> numOfCmd;
    std::map<uint8_t, uint64_t> receivedAcks;


    std::vector<SlaveDescription_t*> slaveDescriptions;

    uint8_t slavesWaitingForConfigure = 0;
    uint8_t slavesWaitingForStep = 0;
    uint8_t slavesWaitingForConfiguration = 0;
    uint8_t slavesWaitingAtExit = 0;
    uint8_t slavesWaitingForDeregister = 0;
    uint8_t slavesWaitingForInitialize = 0;
    uint8_t slavesWaitingToRun = 0;
    uint8_t stoppedSlaves = 0;
};

#endif // DCPMASTER_H
