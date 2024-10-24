#ifndef DCPMASTER_H
#define DCPMASTER_H

#include "hopsandcp_win32dll.h"

#include "HopsanEssentials.h"
#include "ComponentUtilities/num2string.hpp"

#include <vector>
#include <map>

using namespace std;

class SlaveDescription_t;
class DcpManagerMaster;
class UdpDriver;
class OstreamLog;
enum class DcpState : uint8_t;
enum class DcpError : uint16_t;

struct DcpConnection
{
    size_t fromServer, fromVr;
    std::vector<size_t> toServers, toVrs;
};

HOPSANDCP_DLLAPI class DcpMaster
{
public:
    DcpMaster(hopsan::ComponentSystem *pSystem, const string host, int port, double comStep=0.001, double startTime=0, double stopTime=10, bool realTime=false);
    ~DcpMaster();

    void addServer(const string filepath);
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
    void timer(hopsan::ComponentSystem *pSystem, double startTime, double stopTime);

    hopsan::ComponentSystem *mpSystem;
    bool mRealTime;

    std::vector<DcpConnection> connections;

    uint8_t maxInitRuns = 0;
    uint8_t intializationRuns = 1;

    std::map<uint8_t, DcpState> curState;

    double mComStep;
    double mStartTime;
    double mStopTime;

    UdpDriver *mpDriver;

    OstreamLog *mpStdLog;

    DcpManagerMaster *mpManager;

    std::map<uint8_t, uint8_t> numOfCmd;
    std::map<uint8_t, uint64_t> receivedAcks;


    std::vector<SlaveDescription_t*> serverDescriptions;

    uint8_t serversWaitingForConfigure = 0;
    uint8_t serversWaitingForStep = 0;
    uint8_t serversWaitingForConfiguration = 0;
    uint8_t serversWaitingAtExit = 0;
    uint8_t serversWaitingToStop = 0;
    uint8_t serversRunPastStopTime = 0;
    uint8_t serversWaitingForInitialize = 0;
    uint8_t serversWaitingToRun = 0;
};

void getDataFromProtocolFile(const hopsan::HString &rFilePath, hopsan::HString &rName, hopsan::HString &rVariables, hopsan::HString &rValueReferences);


#endif // DCPMASTER_H
