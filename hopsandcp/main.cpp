#include "hopsandcp.h"

#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "HopsanCoreVersion.h"

#include "dcp/helper/Helper.hpp"
#include "dcp/log/OstreamLog.hpp"
#include "dcp/logic/DcpManagerSlave.hpp"
#include "dcp/xml/DcpSlaveDescriptionWriter.hpp"
#include "dcp/xml/DcpSlaveDescriptionReader.hpp"
#include "dcp/model/pdu/DcpPduFactory.hpp"
#include "dcp/driver/ethernet/udp/UdpDriver.hpp"

#include <tclap/CmdLine.h>

#include <iostream>

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE SHAREDLIB_PREFIX "defaultcomponentlibrary" HOPSAN_DEBUG_POSTFIX "." SHAREDLIB_SUFFIX
const std::string default_library = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
#else
const std::string default_library = "";
#endif

using namespace std;
using namespace hopsan;

HopsanEssentials gHopsanCore;
std::string gHost = "127.0.0.1";
int gPort = 8080;

void printWaitingMessages()
{
    hopsan::HString msg, type, tag;
    while (gHopsanCore.checkMessage() > 0)
    {
        gHopsanCore.getMessage(msg,type,tag);
        if(type != "debug") {
            cout << msg.c_str() << endl;
        }
    }
}


SlaveDescription_t getSlaveDescription(std::string name, double timeStep, vector<string> &inputs, vector<string> &outputs){
    SlaveDescription_t slaveDescription = make_SlaveDescription(1, 0, name, "b5279485-720d-4542-9f29-bee4d9a75ef9");
    slaveDescription.OpMode.HardRealTime = make_HardRealTime_ptr();
    slaveDescription.OpMode.SoftRealTime = make_SoftRealTime_ptr();
    slaveDescription.OpMode.NonRealTime = make_NonRealTime_ptr();
    Resolution_t resolution = make_Resolution();
    resolution.numerator = 1;
    resolution.denominator = denominator_t(1/timeStep);
    slaveDescription.TimeRes.resolutions.push_back(resolution);
    slaveDescription.TransportProtocols.UDP_IPv4 = make_UDP_ptr();
    slaveDescription.TransportProtocols.UDP_IPv4->Control = make_Control_ptr(gHost.c_str(), port_t(gPort));
    slaveDescription.TransportProtocols.UDP_IPv4->DAT_input_output = make_DAT_ptr();
    slaveDescription.TransportProtocols.UDP_IPv4->DAT_input_output->availablePortRanges.push_back(make_AvailablePortRange(2048, 65535));
    slaveDescription.TransportProtocols.UDP_IPv4->DAT_parameter = make_DAT_ptr();
    slaveDescription.TransportProtocols.UDP_IPv4->DAT_parameter->availablePortRanges.push_back(make_AvailablePortRange(2048, 65535));
    slaveDescription.CapabilityFlags.canAcceptConfigPdus = true;
    slaveDescription.CapabilityFlags.canHandleReset = false;
    slaveDescription.CapabilityFlags.canHandleVariableSteps = false;
    slaveDescription.CapabilityFlags.canMonitorHeartbeat = false;
    slaveDescription.CapabilityFlags.canProvideLogOnRequest = true;
    slaveDescription.CapabilityFlags.canProvideLogOnNotification = true;

    for(size_t i=0; i<outputs.size(); ++i) {
        std::shared_ptr<Output_t> causality = make_Output_ptr<float64_t>();
        slaveDescription.Variables.push_back(make_Variable_output(outputs[i], valueReference_t(i), causality));
    }
    for(size_t i=0; i<inputs.size(); ++i) {
        std::shared_ptr<CommonCausality_t> causality = make_CommonCausality_ptr<float64_t>();
        causality->Float64->start = std::make_shared<std::vector<float64_t>>();
        causality->Float64->start->push_back(0.0);
        slaveDescription.Variables.push_back(make_Variable_input(inputs[i], valueReference_t(i), causality));
    }

    slaveDescription.Log = make_Log_ptr();
    slaveDescription.Log->categories.push_back(make_Category(1, "DCP_SLAVE"));

    return slaveDescription;
}


void generateDescriptionFile(std::string &modelFile, std::string &targetFile) {
    std::cout << "Generating " << targetFile << " from model " << modelFile << "\n";

    double startTime=0, stopTime=1;
    ComponentSystem *pHopsanSystem = gHopsanCore.loadHMFModelFile(modelFile.c_str(), startTime, stopTime);
    printWaitingMessages();
    if(pHopsanSystem == nullptr) {
        std::cout << "Failed to load model file: " << modelFile << "\n";
        exit(-1);
    }
    std::vector<std::string> inputs, outputs;
    for(const auto &component : pHopsanSystem->getSubComponents()) {
        if(component->getTypeName() == "SignalInputInterface") {
            inputs.push_back(component->getName().c_str());
        }
        else if(component->getTypeName() == "SignalOutputInterface") {
            outputs.push_back(component->getName().c_str());
        }
    }
    std::cout << "Inputs:";
    for(const auto &input : inputs) {
        std::cout << " " << input;
    }
    std::cout << "\nOutputs:";
    for(const auto &output : outputs) {
        std::cout << " " << output;
    }
    std::cout << "\n";

    std::string systemName = pHopsanSystem->getName().c_str();
    writeDcpSlaveDescription(getSlaveDescription(systemName, pHopsanSystem->getTimestep(), inputs, outputs), targetFile.c_str());
}


int main(int argc, char* argv[])
{
    TCLAP::CmdLine cmd("hopsandcp", ' ', "0.1");

    // Define a value argument and add it to the command line.
    TCLAP::SwitchArg generateDescription("d","description","Generate DCP desciption file",cmd);
    TCLAP::ValueArg<std::string> argModelFile("m","model","Hopsan model file",false, "", "", cmd);
    TCLAP::ValueArg<std::string> argTargetDescriptionFile("t","target","Target file for DCP description",false, "", "", cmd);
    TCLAP::ValueArg<std::string> argHost("a","address","Host address",false,"127.0.0.1","",cmd);
    TCLAP::ValueArg<int> argPort("p","port","Port",false,8080,"",cmd);
    // Parse the argv array.
    cmd.parse( argc, argv );

    gHost = argHost.getValue();
    gPort = argPort.getValue();

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
    // Load default Hopsan component lib
    string libpath = default_library;
    gHopsanCore.loadExternalComponentLib(libpath.c_str());
#endif

    printWaitingMessages();

    if(generateDescription.isSet()) {
        if(!argModelFile.isSet()) {
            cout << "Generating a DCP description requires a model file.\n";
            return -1;
        }
        string modelFile = argModelFile.getValue();
        string targetFile;
        if(argTargetDescriptionFile.isSet()) {
            targetFile = argTargetDescriptionFile.getValue();
        }
        else {
            targetFile = argModelFile.getValue();
            targetFile = targetFile.substr(0,targetFile.find_last_of('.'))+".dcpx";
        }

        generateDescriptionFile(modelFile, targetFile);
    }

    std::cout << "hopsandcp completed successfully!\n";

    return 0;
}
