#ifndef DCPSLAVE_H
#define DCPSLAVE_H

#include <string>
#include <vector>

namespace hopsan {
class HopsanEssentials;
class ComponentSystem;
}

struct SlaveDescription_t;
class DcpManagerSlave;
class UdpDriver;
class OstreamLog;

class DcpSlave
{
public:
    DcpSlave(const std::string modelfile, const std::string host, int port, std::string resultFile="");
    ~DcpSlave();

    void generateDescriptionFile(std::string &targetFile);
    void start();

private:
    void printWaitingMessages();
    SlaveDescription_t *getSlaveDescription();

    hopsan::HopsanEssentials *mpHopsanCore;
    hopsan::ComponentSystem *mpRootSystem;
    std::vector<std::string> mInputs, mOutputs;
    std::vector<double*> mInputNodePtrs, mOutputNodePtrs;
    std::vector<double*> mInputDataPtrs, mOutputDataPtrs;
    std::string mHost = "127.0.0.1";
    int mPort = 8080;
    std::string mResultFile;
    DcpManagerSlave *mManager;
    OstreamLog *stdLog;
    UdpDriver* udpDriver;
    double mSimulationTime=0;

    void configure();
    void initialize();
    void doStep(uint64_t steps);
    void setTimeRes(const uint32_t numerator, const uint32_t denominator);
    void stop();
};

#endif // DCPSLAVE_H
