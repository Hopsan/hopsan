#ifndef DCPSLAVE_H
#define DCPSLAVE_H

#include "hopsandcp_win32dll.h"

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

HOPSANDCP_DLLAPI class DcpSlave
{
public:
    DcpSlave(hopsan::ComponentSystem *pSystem, const std::string host, int port, size_t numLogSamples);
    ~DcpSlave();

    void generateDcpFile(std::string targetFile);
    bool start();

private:
    SlaveDescription_t *getSlaveDescription();

    hopsan::ComponentSystem *mpRootSystem;
    std::vector<std::string> mInputs, mOutputs;
    std::vector<double*> mInputNodePtrs, mOutputNodePtrs;
    std::vector<double*> mInputDataPtrs, mOutputDataPtrs;
    std::string mHost = "127.0.0.1";
    int mPort = 8080;
    size_t mNumLogSamples = 0;
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
