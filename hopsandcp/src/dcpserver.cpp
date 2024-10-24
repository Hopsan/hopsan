#include "dcpserver.h"
#include "ComponentSystem.h"
#include "utilities.h"

#include "dcp/log/OstreamLog.hpp"
#include "dcp/logic/DcpManagerSlave.hpp"
#include "dcp/zip/DcpSlaveWriter.hpp"
#include "dcp/driver/ethernet/udp/UdpDriver.hpp"

//#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "HopsanCoreVersion.h"

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE SHAREDLIB_PREFIX "defaultcomponentlibrary" HOPSAN_DEBUG_POSTFIX "." SHAREDLIB_SUFFIX
const std::string default_library = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
#else
const std::string default_library = "";
#endif

const LogTemplate SIM_LOG = LogTemplate(
        1, 1, DcpLogLevel::LVL_INFORMATION,
        "[Time = %float64]: output = %float64",
        {DcpDataType::float64, DcpDataType::float64});


using namespace hopsan;

DcpServer::DcpServer(ComponentSystem *pSystem, const std::string host, int port, size_t numLogSamples)
    : mpRootSystem(pSystem), mHost(host), mPort(port), mNumLogSamples(numLogSamples)
{
    //Generate list of inputs and outputs based on interface components
    for(const auto &component : pSystem->getSubComponents()) {
        if(component->getTypeName() == "SignalInputInterface") {
            mInputs.push_back(component->getName().c_str());
        }
        else if(component->getTypeName() == "SignalOutputInterface") {
            mOutputs.push_back(component->getName().c_str());
        }
    }
    std::cout << "Inputs:";
    for(const auto &input : mInputs) {
        std::cout << " " << input;
    }
    std::cout << "\nOutputs:";
    for(const auto &output : mOutputs) {
        std::cout << " " << output;
    }
    std::cout << "\n";

    //Create UDP driver
    mpDriver = new UdpDriver(host, uint16_t(port));

    //Create server DCP manager
    mpServerDescription = createServerDescription();
    mManager = new DcpManagerSlave(*mpServerDescription, mpDriver->getDcpDriver());
    mManager->setInitializeCallback<SYNC>(
            std::bind(&DcpServer::initialize, this));
    mManager->setConfigureCallback<SYNC>(
            std::bind(&DcpServer::configure, this));
    mManager->setSynchronizingStepCallback<SYNC>(
            std::bind(&DcpServer::doStep, this, std::placeholders::_1));
    mManager->setSynchronizedStepCallback<SYNC>(
            std::bind(&DcpServer::doStep, this, std::placeholders::_1));
    mManager->setRunningStepCallback<SYNC>(
            std::bind(&DcpServer::doStep, this, std::placeholders::_1));
    mManager->setRunningNRTStepCallback<SYNC>(
                std::bind(&DcpServer::doStep, this, std::placeholders::_1));
    mManager->setTimeResListener<SYNC>(std::bind(&DcpServer::setTimeRes, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
    mManager->setStopCallback<SYNC>(
                std::bind(&DcpServer::stop, this));

    //Display log messages on console
    mpStdLog = new OstreamLog(std::cout);
    mManager->addLogListener(
            std::bind(&OstreamLog::logOstream, *mpStdLog, std::placeholders::_1));
    mManager->setGenerateLogString(true);
}

DcpServer::~DcpServer()
{
}

std::shared_ptr<SlaveDescription_t> DcpServer::createServerDescription() {
    SlaveDescription_t serverDescription = make_SlaveDescription(1, 0, mpRootSystem->getName().c_str(), generateUuid().c_str());
    serverDescription.OpMode.HardRealTime = make_HardRealTime_ptr();
    serverDescription.OpMode.SoftRealTime = make_SoftRealTime_ptr();
    serverDescription.OpMode.NonRealTime = make_NonRealTime_ptr();
    Resolution_t resolution = make_Resolution();
    resolution.numerator = 1;
    resolution.denominator = denominator_t(1.0/mpRootSystem->getTimestep());
    resolution.fixed = false;
    serverDescription.TimeRes.resolutions.push_back(resolution);
    serverDescription.TransportProtocols.UDP_IPv4 = make_UDP_ptr();
    serverDescription.TransportProtocols.UDP_IPv4->Control = make_Control_ptr(mHost.c_str(), port_t(mPort));
    serverDescription.TransportProtocols.UDP_IPv4->DAT_input_output = make_DAT_ptr();
    serverDescription.TransportProtocols.UDP_IPv4->DAT_input_output->availablePortRanges.push_back(make_AvailablePortRange(2048, 65535));
    serverDescription.TransportProtocols.UDP_IPv4->DAT_parameter = make_DAT_ptr();
    serverDescription.TransportProtocols.UDP_IPv4->DAT_parameter->availablePortRanges.push_back(make_AvailablePortRange(2048, 65535));
    serverDescription.CapabilityFlags.canAcceptConfigPdus = true;
    serverDescription.CapabilityFlags.canHandleReset = false;
    serverDescription.CapabilityFlags.canHandleVariableSteps = false;
    serverDescription.CapabilityFlags.canMonitorHeartbeat = false;
    serverDescription.CapabilityFlags.canProvideLogOnRequest = true;
    serverDescription.CapabilityFlags.canProvideLogOnNotification = true;

    for(size_t i=0; i<mOutputs.size(); ++i) {
        std::shared_ptr<Output_t> causality = make_Output_ptr<float64_t>();
        serverDescription.Variables.push_back(make_Variable_output(mOutputs[i], valueReference_t(i), causality));
    }
    for(size_t i=0; i<mInputs.size(); ++i) {
        std::shared_ptr<CommonCausality_t> causality = make_CommonCausality_ptr<float64_t>();
        causality->Float64->start = std::make_shared<std::vector<float64_t>>();
        causality->Float64->start->push_back(0.0);
        serverDescription.Variables.push_back(make_Variable_input(mInputs[i], valueReference_t(mOutputs.size()+i), causality));
    }

    serverDescription.Log = make_Log_ptr();
    serverDescription.Log->categories.push_back(make_Category(1, "DCP_SERVER"));
    serverDescription.Log->templates.push_back(make_Template(
            1, 1, (uint8_t) DcpLogLevel::LVL_INFORMATION, "[Time = %float64]: output = %float64"));

    return std::make_shared<SlaveDescription_t>(serverDescription);
}


void DcpServer::generateDcpFile(std::string targetFile) {
    writeDcpSlaveFile(std::shared_ptr<SlaveDescription_t>(mpServerDescription), targetFile.c_str());
}

bool DcpServer::start()
{
    try {
        mManager->start();
        return true;
    } catch (std::exception& e) {
        mpRootSystem->addErrorMessage(e.what());
        return false;
    }
}


void DcpServer::configure() {
    std::cout << "Checking model... ";
    if (mpRootSystem->checkModelBeforeSimulation()) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        exit(-1);
    }

    for(size_t o=0; o<mOutputs.size(); ++o) {
        mOutputDataPtrs.push_back(mManager->getOutput<float64_t *>(o));
        mOutputNodePtrs.push_back(mpRootSystem->getSubComponent(mOutputs[o].c_str())->getPort("in")->getNodeDataPtr(0));
    }
    for(size_t i=0; i<mInputs.size(); ++i) {
        mInputDataPtrs.push_back(mManager->getInput<float64_t *>(mOutputs.size()+i));
        mInputNodePtrs.push_back(mpRootSystem->getSubComponent(mInputs[i].c_str())->getPort("out")->getNodeDataPtr(0));
    }

    std::cout << "Initializing... ";
    mpRootSystem->setNumLogSamples(mNumLogSamples);
    mpRootSystem->setLogStartTime(0);
    if(mpRootSystem->initialize(0,10)) {
        std::cout << "Success!\n";
    }
    else {
        std::cout << "Failed!\n";
        exit(-1);
    }
}

void DcpServer::initialize() {


}

void DcpServer::doStep(uint64_t steps) {

    // Read inputs
    for(size_t i=0; i<mInputs.size(); ++i) {
        *(mInputNodePtrs[i]) = *(mInputDataPtrs[i]);
    }

    //Simulate
    mSimulationTime += steps*mpRootSystem->getTimestep();
    mpRootSystem->simulate(mSimulationTime);

     // Write outputs
    for(size_t o=0; o<mOutputs.size(); ++o) {
        *(mOutputDataPtrs[o]) = *(mOutputNodePtrs[o]);
        mManager->Log(SIM_LOG, mSimulationTime, *mOutputDataPtrs[o]);
    }
}

void DcpServer::setTimeRes(const uint32_t numerator, const uint32_t denominator) {
    mpRootSystem->setDesiredTimestep(double(numerator)/double(denominator));
}

void DcpServer::stop()
{
    //dynamic_cast<AbstractDcpManager*>(mManager)->stop();
    mpDriver->getDcpDriver().stopReceiving();
    mpDriver->getDcpDriver().disconnect();
}
