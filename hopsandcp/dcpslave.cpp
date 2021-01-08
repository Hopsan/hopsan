#include "dcpslave.h"

#include "dcp/logic/DcpManagerSlave.hpp"
#include "dcp/xml/DcpSlaveDescriptionWriter.hpp"

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

using namespace hopsan;

DcpSlave::DcpSlave(const std::string modelfile, const std::string host, int port)
    : mHost(host), mPort(port)
{
    //Create Hopsan object
    mpHopsanCore = new HopsanEssentials();

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
    // Load default Hopsan component lib
    std::string libpath = default_library;
    mpHopsanCore->loadExternalComponentLib(libpath.c_str());
#endif

    //Load model file
    double startTime=0, stopTime=1;
    mpRootSystem = mpHopsanCore->loadHMFModelFile(modelfile.c_str(), startTime, stopTime);
    printWaitingMessages();
    if(mpRootSystem == nullptr) {
        std::cout << "Failed to load model file: " << modelfile << "\n";
        exit(-1);
    }

    //Generate list of inputs and outputs based on interface components
    for(const auto &component : mpRootSystem->getSubComponents()) {
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

    printWaitingMessages();
}

DcpSlave::~DcpSlave()
{
    delete mpHopsanCore;
}


void DcpSlave::printWaitingMessages()
{
    hopsan::HString msg, type, tag;
    while (mpHopsanCore->checkMessage() > 0)
    {
        mpHopsanCore->getMessage(msg,type,tag);
        if(type != "debug") {
            std::cout << msg.c_str() << std::endl;
        }
    }
}


SlaveDescription_t *DcpSlave::getSlaveDescription() {
    SlaveDescription_t *slaveDescription = new SlaveDescription_t(make_SlaveDescription(1, 0, mpRootSystem->getName().c_str(), "b5279485-720d-4542-9f29-bee4d9a75ef9"));
    slaveDescription->OpMode.HardRealTime = make_HardRealTime_ptr();
    slaveDescription->OpMode.SoftRealTime = make_SoftRealTime_ptr();
    slaveDescription->OpMode.NonRealTime = make_NonRealTime_ptr();
    Resolution_t resolution = make_Resolution();
    resolution.numerator = 1;
    resolution.denominator = denominator_t(1/mpRootSystem->getTimestep());
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
        slaveDescription->Variables.push_back(make_Variable_input(mInputs[i], valueReference_t(i), causality));
    }

    slaveDescription->Log = make_Log_ptr();
    slaveDescription->Log->categories.push_back(make_Category(1, "DCP_SLAVE"));

    return slaveDescription;
}


void DcpSlave::generateDescriptionFile(std::string &targetFile) {
    writeDcpSlaveDescription(*getSlaveDescription(), targetFile.c_str());
}
