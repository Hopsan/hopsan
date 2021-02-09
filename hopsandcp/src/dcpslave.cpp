#include "dcpslave.h"
#include "utilities.h"

#include <dcp/helper/Helper.hpp>
#include <dcp/log/OstreamLog.hpp>
#include <dcp/logic/DcpManagerSlave.hpp>
#include <dcp/xml/DcpSlaveDescriptionWriter.hpp>
#include <dcp/model/pdu/DcpPduFactory.hpp>
#include <dcp/driver/ethernet/udp/UdpDriver.hpp>

#include "HopsanEssentials.h"
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

DcpSlave::DcpSlave(ComponentSystem *pSystem, const std::string host, int port, size_t numLogSamples)
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
    udpDriver = new UdpDriver(host, uint16_t(port));

    //Create slave DCP manager
    mManager = new DcpManagerSlave(*getSlaveDescription(), udpDriver->getDcpDriver());
    mManager->setInitializeCallback<SYNC>(
            std::bind(&DcpSlave::initialize, this));
    mManager->setConfigureCallback<SYNC>(
            std::bind(&DcpSlave::configure, this));
    mManager->setSynchronizingStepCallback<SYNC>(
            std::bind(&DcpSlave::doStep, this, std::placeholders::_1));
    mManager->setSynchronizedStepCallback<SYNC>(
            std::bind(&DcpSlave::doStep, this, std::placeholders::_1));
    mManager->setRunningStepCallback<SYNC>(
            std::bind(&DcpSlave::doStep, this, std::placeholders::_1));
    mManager->setRunningNRTStepCallback<SYNC>(
                std::bind(&DcpSlave::doStep, this, std::placeholders::_1));
    mManager->setTimeResListener<SYNC>(std::bind(&DcpSlave::setTimeRes, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
    mManager->setStopCallback<SYNC>(
                std::bind(&DcpSlave::stop, this));

    //Display log messages on console
    stdLog = new OstreamLog(std::cout);
    mManager->addLogListener(
            std::bind(&OstreamLog::logOstream, *stdLog, std::placeholders::_1));
    mManager->setGenerateLogString(true);
}

DcpSlave::~DcpSlave()
{
}

SlaveDescription_t *DcpSlave::getSlaveDescription() {
    SlaveDescription_t *slaveDescription = new SlaveDescription_t(make_SlaveDescription(1, 0, mpRootSystem->getName().c_str(), "b5279485-720d-4542-9f29-bee4d9a75ef9"));
    slaveDescription->OpMode.HardRealTime = make_HardRealTime_ptr();
    slaveDescription->OpMode.SoftRealTime = make_SoftRealTime_ptr();
    slaveDescription->OpMode.NonRealTime = make_NonRealTime_ptr();
    Resolution_t resolution = make_Resolution();
    resolution.numerator = 1;
    resolution.denominator = denominator_t(1.0/mpRootSystem->getTimestep());
    slaveDescription->TimeRes.resolutions.push_back(resolution);
    slaveDescription->TransportProtocols.UDP_IPv4 = make_UDP_ptr();
    slaveDescription->TransportProtocols.UDP_IPv4->Control = make_Control_ptr(mHost.c_str(), port_t(mPort));
    slaveDescription->TransportProtocols.UDP_IPv4->DAT_input_output = make_DAT_ptr();
    slaveDescription->TransportProtocols.UDP_IPv4->DAT_input_output->availablePortRanges.push_back(make_AvailablePortRange(2048, 65535));
    slaveDescription->TransportProtocols.UDP_IPv4->DAT_parameter = make_DAT_ptr();
    slaveDescription->TransportProtocols.UDP_IPv4->DAT_parameter->availablePortRanges.push_back(make_AvailablePortRange(2048, 65535));
    slaveDescription->CapabilityFlags.canAcceptConfigPdus = true;
    slaveDescription->CapabilityFlags.canHandleReset = false;
    slaveDescription->CapabilityFlags.canHandleVariableSteps = false;
    slaveDescription->CapabilityFlags.canMonitorHeartbeat = false;
    slaveDescription->CapabilityFlags.canProvideLogOnRequest = true;
    slaveDescription->CapabilityFlags.canProvideLogOnNotification = true;

    for(size_t i=0; i<mOutputs.size(); ++i) {
        std::shared_ptr<Output_t> causality = make_Output_ptr<float64_t>();
        slaveDescription->Variables.push_back(make_Variable_output(mOutputs[i], valueReference_t(i), causality));
    }
    for(size_t i=0; i<mInputs.size(); ++i) {
        std::shared_ptr<CommonCausality_t> causality = make_CommonCausality_ptr<float64_t>();
        causality->Float64->start = std::make_shared<std::vector<float64_t>>();
        causality->Float64->start->push_back(0.0);
        slaveDescription->Variables.push_back(make_Variable_input(mInputs[i], valueReference_t(mOutputs.size()+i), causality));
    }

    slaveDescription->Log = make_Log_ptr();
    slaveDescription->Log->categories.push_back(make_Category(1, "DCP_SLAVE"));
    slaveDescription->Log->templates.push_back(make_Template(
            1, 1, (uint8_t) DcpLogLevel::LVL_INFORMATION, "[Time = %float64]: output = %float64"));

    return slaveDescription;
}


void DcpSlave::generateDescriptionFile(std::string targetFile) {
    writeDcpSlaveDescription(*getSlaveDescription(), targetFile.c_str());
}

bool DcpSlave::start()
{
    try {
        mManager->start();
        return true;
    } catch (std::exception& e) {
        mpRootSystem->addErrorMessage(e.what());
        return false;
    }
}


void DcpSlave::configure() {
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

void DcpSlave::initialize() {


}

void DcpSlave::doStep(uint64_t steps) {

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

void DcpSlave::setTimeRes(const uint32_t numerator, const uint32_t denominator) {
    mpRootSystem->setDesiredTimestep(double(numerator)/double(denominator));
}

void DcpSlave::stop()
{
    dynamic_cast<AbstractDcpManager*>(mManager)->stop();
}
