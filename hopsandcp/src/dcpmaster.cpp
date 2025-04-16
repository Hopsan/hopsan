#include "dcpmaster.h"

#include "dcp/log/OstreamLog.hpp"
#include "dcp/helper/LogHelper.hpp"
//#include "dcp/model/pdu/DcpPduFactory.hpp"
#include "dcp/zip/DcpSlaveReader.hpp"
#include "dcp/driver/ethernet/udp/UdpDriver.hpp"
#include "dcp/logic/DcpManagerMaster.hpp"

#include <cmath>
#include <iostream>
#include <cstdint>
//#include <fstream>
#include <memory>
#include <chrono>

#include "ComponentUtilities/num2string.hpp"

DcpMaster::DcpMaster(hopsan::ComponentSystem *pSystem, const std::string host, int port, double comStep, double startTime, double stopTime, bool realTime)
    : mpSystem(pSystem), mRealTime(realTime), mComStep(comStep), mStartTime(startTime), mStopTime(stopTime)
{
    (*mpSystem->getTimePtr()) = mStartTime;

    OstreamLog stdLog(std::cout);

    mpDriver = new UdpDriver(host, port_t(port));

    mpManager = new DcpManagerMaster(mpDriver->getDcpDriver());

    mpManager->setAckReceivedListener<SYNC>(
                std::bind(&DcpMaster::receiveAck, this, std::placeholders::_1, std::placeholders::_2));
    mpManager->setNAckReceivedListener<SYNC>(
                std::bind(&DcpMaster::receiveNAck, this, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3));
    mpManager->setStateChangedNotificationReceivedListener<SYNC>(
                std::bind(&DcpMaster::receiveStateChangedNotification, this, std::placeholders::_1,
                          std::placeholders::_2));
    mpManager->setDataReceivedListener<SYNC>(
                std::bind(&DcpMaster::dataReceived, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    mpManager->addLogListener(std::bind(&OstreamLog::logOstream, stdLog, std::placeholders::_1));
    mpManager->setGenerateLogString(true);
}

DcpMaster::~DcpMaster() {
    delete mpDriver;
    delete mpManager;
}

void DcpMaster::addServer(const string filepath)
{
    serverDescriptions.push_back(new SlaveDescription_t(*getSlaveDescriptionFromDcpFile(1,0,filepath.c_str())));
    u_char id = u_char(serverDescriptions.size());
    uint8_t *netInfo = new uint8_t[6];
    *((uint16_t *) netInfo) = *serverDescriptions.back()->TransportProtocols.UDP_IPv4->Control->port;
    *((uint32_t *) (netInfo + 2)) = asio::ip::address_v4::from_string(*serverDescriptions.back()->TransportProtocols.UDP_IPv4->Control->host).to_ulong();
    mpDriver->getDcpDriver().setSlaveNetworkInformation(id, netInfo);
    delete[] netInfo;
}

void DcpMaster::addConnection(size_t fromServer, size_t fromVr, std::vector<size_t> toServers, std::vector<size_t> toVrs)
{
    DcpConnection connection;
    connection.fromServer = fromServer;
    connection.fromVr = fromVr;
    connection.toVrs = toVrs;
    connection.toServers = toServers;
    connections.push_back(connection);
}


void DcpMaster::start() {
    std::thread b(&DcpManagerMaster::start, mpManager);
    std::chrono::seconds dura(1);
    std::this_thread::sleep_for(dura);
    DcpOpMode mode = DcpOpMode::NRT;
    if(mRealTime) {
        mode = DcpOpMode::SRT;
    }

    for(size_t i=0; i<serverDescriptions.size(); ++i) {
        mpManager->STC_register(u_char(i+1), DcpState::ALIVE, convertToUUID(serverDescriptions[i]->uuid), mode, 1, 0);
    }
    b.join();
}

void DcpMaster::initialize() {
    serversWaitingForInitialize++;
    if(serversWaitingForInitialize < serverDescriptions.size()) {
        return;
    }
    for(size_t i=0; i<serverDescriptions.size(); ++i) {
        mpManager->STC_initialize(u_char(i), DcpState::CONFIGURED);
    }
    intializationRuns++;
}

void DcpMaster::configuration() {
    serversWaitingForConfiguration++;
    if(serversWaitingForConfiguration < serverDescriptions.size()) {
        return;
    }

    //Count received acks so that we know when all configuration messages are acknowledged
    for(size_t i=1; i<=serverDescriptions.size(); ++i) {
        receivedAcks[dcpId_t(i)] = 0;
        numOfCmd[dcpId_t(i)] = 0;

        mpManager->CFG_time_res(uint8_t(i), 1, uint32_t(std::floor(1.0/mComStep)));
        numOfCmd[dcpId_t(i)]++;
    }

    for(size_t i=0; i<connections.size(); ++i) {
        uint16_t dataId = uint16_t(i+1);
        uint8_t fromServerId = uint8_t(connections[i].fromServer);
        mpManager->CFG_scope(fromServerId, dataId, DcpScope::Initialization_Run_NonRealTime);
        mpManager->CFG_output(fromServerId, dataId, 0, connections[i].fromVr);
        mpManager->CFG_steps(fromServerId, dataId, 1);
        numOfCmd[fromServerId] += 4;

        for(size_t j=0; j<connections[i].toServers.size(); ++j) {
            uint8_t toServerId = uint8_t(connections[i].toServers[j]);
            mpManager->CFG_scope(toServerId, dataId, DcpScope::Initialization_Run_NonRealTime);
            mpManager->CFG_input(toServerId, dataId, 0, connections[i].toVrs[j], DcpDataType::float64);
            mpManager->CFG_steps(toServerId, dataId, 1);
            mpManager->CFG_source_network_information_UDP(toServerId, dataId, asio::ip::address_v4::from_string(*serverDescriptions[toServerId-1]->TransportProtocols.UDP_IPv4->Control->host).to_uint(), *serverDescriptions[toServerId-1]->TransportProtocols.UDP_IPv4->Control->port+1);
            mpManager->CFG_target_network_information_UDP(fromServerId, dataId,  asio::ip::address_v4::from_string(*serverDescriptions[toServerId-1]->TransportProtocols.UDP_IPv4->Control->host).to_uint(), *serverDescriptions[toServerId-1]->TransportProtocols.UDP_IPv4->Control->port);
            numOfCmd[toServerId] += 4;
        }
    }
}

void DcpMaster::configure() {
    serversWaitingForConfigure++;
    if(serversWaitingForConfigure < serverDescriptions.size()) {
        return;
    }
    for(size_t i=0; i<serverDescriptions.size(); ++i) {
        mpManager->STC_configure(u_char(i+1), DcpState::PREPARED);
    }
}

void DcpMaster::run(DcpState currentState, uint8_t sender) {
    std::time_t now = std::time(nullptr);
    mpManager->STC_run(sender, currentState, now + 2);
}

void DcpMaster::doStep() {
    serversWaitingForStep++;
    if(serversWaitingForStep < serverDescriptions.size()) {
        return;
    }
    serversWaitingForStep = 0;
    for(size_t i=0; i<serverDescriptions.size(); ++i) {
        mpManager->STC_do_step(u_char(i+1),DcpState::RUNNING,1);
    }

    (*mpSystem->getTimePtr()) += mComStep;
}

void DcpMaster::stop() {
    std::chrono::seconds dura(1);
    std::this_thread::sleep_for(dura);
    for(size_t i=0; i<serverDescriptions.size(); ++i) {
        mpManager->STC_stop(u_char(i+1), DcpState::RUNNING);
    }
}

void DcpMaster::deregister() {
    serversWaitingToStop++;
    if(serversWaitingToStop < serverDescriptions.size()) {
        return;
    }
    for(size_t i=0; i<serverDescriptions.size(); ++i) {
        mpManager->STC_deregister(u_char(i+1), DcpState::STOPPED);
    }
}

void DcpMaster::sendOutputs(DcpState currentState, uint8_t sender) {
    mpManager->STC_send_outputs(sender, currentState);
}

void DcpMaster::receiveAck(uint8_t sender, uint16_t pduSeqId) {
    (void)pduSeqId;
    receivedAcks[sender]++;
    if (receivedAcks[sender] == numOfCmd[sender]) {
        mpManager->STC_prepare(sender, DcpState::CONFIGURATION);
    }
}

void DcpMaster::receiveNAck(uint8_t sender, uint16_t pduSeqId, DcpError errorCode) {
    (void)sender;
    (void)pduSeqId;
    (void)errorCode;
    std::exit(1);
}



void DcpMaster::dataReceived(uint16_t dataId, size_t length, uint8_t payload[]) {
    (void)dataId;
    (void)length;
    (void)payload;
}

void DcpMaster::receiveStateChangedNotification(uint8_t sender,
                                                DcpState state) {
    switch (state) {
        case DcpState::CONFIGURATION:
            configuration();
            break;

        case DcpState::CONFIGURED:
            if (intializationRuns < maxInitRuns) {
                initialize();

            } else {
                run(DcpState::CONFIGURED, sender);
            }
            break;

        case DcpState::SYNCHRONIZED:
            run(DcpState::SYNCHRONIZED, sender);
            break;

        case DcpState::PREPARED:
            configure();
            break;

        case DcpState::INITIALIZED:
            sendOutputs(DcpState::INITIALIZED, sender);
            break;

        case DcpState::RUNNING:
            if(mpSystem->getTime() > mStopTime) {
                serversRunPastStopTime++;
                if(serversRunPastStopTime < serverDescriptions.size()) {
                    return;
                }
                stop();
            }
            else {
                if(!mRealTime) {
                    doStep();
                }
                else {
                    std::thread timerThread(&DcpMaster::timer, this, mpSystem, mStartTime, mStopTime);
                    timerThread.join();
                    serversWaitingToStop = uint8_t(serverDescriptions.size());
                    stop();
                }
            }
            break;

        case DcpState::COMPUTED:
            sendOutputs(DcpState::COMPUTED, sender);
            break;

        case DcpState::STOPPED:
            serversWaitingToStop++;
            if(serversWaitingToStop < serverDescriptions.size()) {
                return;
            }
            mpManager->stop();
            break;

        case DcpState::ALIVE:
            //simulation finished
            serversWaitingAtExit++;
            if(serversWaitingAtExit < serverDescriptions.size()) {
                return;
            }
            break;

        case DcpState::PREPARING:
        default:
            break;
    }
}

void DcpMaster::timer(hopsan::ComponentSystem *pSystem, double startTime, double stopTime)
{
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    double simTime = startTime;
    while(simTime < stopTime) {
        time += std::chrono::milliseconds{100};
        simTime += 0.1;
        (*pSystem->getTimePtr()) = simTime;
        std::this_thread::sleep_until(time);
    }
}



void getDataFromProtocolFile(const hopsan::HString &rFilePath, hopsan::HString &rName, hopsan::HString &rVariables, hopsan::HString &rValueReferences)
{
    shared_ptr<SlaveDescription_t> desc = getSlaveDescriptionFromDcpFile(1,0,rFilePath.c_str());
    hopsan::HString inputs,outputs,pars;
    hopsan::HString inputVrs, outputVrs, parVrs;
    for(const auto &var : desc->Variables) {
        if(var.Input.get() != nullptr) {
            inputs.append(hopsan::HString(var.name.c_str())+",");
            inputVrs.append(to_hstring(var.valueReference)+",");
        }
        else if(var.Output.get() != nullptr) {
            outputs.append(hopsan::HString(var.name.c_str())+",");
            outputVrs.append(to_hstring(var.valueReference)+",");
        }
        else if(var.Parameter.get() != nullptr) {
            pars.append(hopsan::HString(var.name.c_str())+",");
            parVrs.append(to_hstring(var.valueReference)+",");
        }
    }
    if(!inputs.empty()) {
        inputs.erase(inputs.size()-1);
        inputVrs.erase(inputVrs.size()-1);
    }
    if(!outputs.empty()) {
        outputs.erase(outputs.size()-1);
        outputVrs.erase(outputVrs.size()-1);
    }
    if(!pars.empty()) {
        pars.erase(pars.size()-1);
        parVrs.erase(parVrs.size()-1);
    }
    rName = desc->dcpSlaveName.c_str();

    //Variables names are stored in a semicolon and comma separated string.
    //Variable types are separated by semicolons, while variable names within
    //each type are separated by commas.
    rVariables = inputs+";"+outputs+";"+pars;

    //Value references is a plain comma-separated string
    hopsan::HVector<hopsan::HString> valueRefVec = inputVrs.split(',', hopsan::HString::SkipEmptyParts);
    valueRefVec.append(outputVrs.split(',', hopsan::HString::SkipEmptyParts));
    valueRefVec.append(parVrs.split(',', hopsan::HString::SkipEmptyParts));
    rValueReferences = join(valueRefVec, ',');

    //Value references string could end with a comma if one
    //of the lists above is empty. If so, remove this.
    if(rValueReferences.at(rValueReferences.size()-1) == ',') {
        rValueReferences.erase(rValueReferences.size()-1);
    }
}
