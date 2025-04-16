#ifndef DcpServer_H
#define DcpServer_H

#include "hopsandcp_win32dll.h"

#include <string>
#include <vector>
#include <memory>

namespace hopsan {
class HopsanEssentials;
class ComponentSystem;
}

struct SlaveDescription_t;
class DcpManagerSlave;
class UdpDriver;
class OstreamLog;

HOPSANDCP_DLLAPI class DcpServer
{
public:
    DcpServer(hopsan::ComponentSystem *pSystem, const std::string host, int port, size_t numLogSamples);
    ~DcpServer();

    void generateDcpFile(std::string targetFile);
    bool start();

private:
    std::shared_ptr<SlaveDescription_t> createServerDescription();

    std::shared_ptr<SlaveDescription_t> mpServerDescription;
    hopsan::ComponentSystem *mpRootSystem;
    std::vector<std::string> mInputs, mOutputs;
    std::vector<double*> mInputNodePtrs, mOutputNodePtrs;
    std::vector<double*> mInputDataPtrs, mOutputDataPtrs;
    std::string mHost = "127.0.0.1";
    int mPort = 8080;
    size_t mNumLogSamples = 0;
    DcpManagerSlave *mManager;
    OstreamLog *mpStdLog;
    UdpDriver* mpDriver;
    double mSimulationTime=0;
    double mStepTime = 0.01; //! @todo Make adjustable!

    void configure();
    void initialize();
    void doStep(uint64_t steps);
    void setTimeRes(const uint32_t numerator, const uint32_t denominator);
    void stop();
};

#endif // DcpServer_H
