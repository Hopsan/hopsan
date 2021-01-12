#include "dcpmaster.h"

#include <iostream>
#include <cstdint>
#include <fstream>
#include <memory>

#include "dcp/helper/LogHelper.hpp"
#include "dcp/model/pdu/DcpPduFactory.hpp"
#include "dcp/xml/DcpSlaveDescriptionReader.hpp"
#include "dcp/driver/ethernet/udp/UdpDriver.hpp"
#include "dcp/logic/DcpManagerMaster.hpp"


DcpMaster::DcpMaster(const std::string host, u_short port)
    : stdLog(std::cout)
{
    driver = new UdpDriver(host, port);

    manager = new DcpManagerMaster(driver->getDcpDriver());

    manager->setAckReceivedListener<SYNC>(
                std::bind(&DcpMaster::receiveAck, this, std::placeholders::_1, std::placeholders::_2));
    manager->setNAckReceivedListener<SYNC>(
                std::bind(&DcpMaster::receiveNAck, this, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3));
    manager->setStateChangedNotificationReceivedListener<SYNC>(
                std::bind(&DcpMaster::receiveStateChangedNotification, this, std::placeholders::_1,
                          std::placeholders::_2));
    manager->setDataReceivedListener<SYNC>(
                std::bind(&DcpMaster::dataReceived, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    manager->addLogListener(std::bind(&OstreamLog::logOstream, stdLog, std::placeholders::_1));
    manager->setGenerateLogString(true);
}

DcpMaster::~DcpMaster() {
    delete driver;
    delete manager;
}

void DcpMaster::addSlave(string &filepath)
{
    slaveDescriptions.push_back(new SlaveDescription_t(*readSlaveDescription(filepath.c_str())));
    u_char id = u_char(slaveDescriptions.size());
    uint8_t *netInfo = new uint8_t[6];
    //*((uint8_t *) netInfo) = *slaveDescriptions.back()->TransportProtocols.UDP_IPv4->Control->port;
    *((uint16_t *) netInfo) = *slaveDescriptions.back()->TransportProtocols.UDP_IPv4->Control->port;
    //*((uint8_t *) (netInfo + 2)) = asio::ip::address_v4::from_string(*slaveDescriptions.back()->TransportProtocols.UDP_IPv4->Control->host).to_uint();
    *((uint32_t *) (netInfo + 2)) = asio::ip::address_v4::from_string(*slaveDescriptions.back()->TransportProtocols.UDP_IPv4->Control->host).to_ulong();
    driver->getDcpDriver().setSlaveNetworkInformation(id, netInfo);
    delete[] netInfo;
}

void DcpMaster::addConnection(size_t fromSlave, size_t fromVr, std::vector<size_t> toSlaves, std::vector<size_t> toVrs)
{
    std::cout << "addConnection: " << fromSlave << "," << fromVr << "," << toSlaves[0] << "," << toVrs[0] << "\n";
    DcpConnection connection;
    connection.fromSlave = fromSlave;
    connection.fromVr = fromVr;
    connection.toVrs = toVrs;
    connection.toSlaves = toSlaves;
    connections.push_back(connection);
}

void DcpMaster::start() {
    std::thread b(&DcpManagerMaster::start, manager);
    std::chrono::seconds dura(1);
    std::this_thread::sleep_for(dura);
    //driver->getDcpDriver().connectToSlave(1);
    std::cout << "Register Slaves" << std::endl;
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        manager->STC_register(u_char(i+1), DcpState::ALIVE, convertToUUID(slaveDescriptions[i]->uuid), DcpOpMode::NRT, 1, 0);
    }
    b.join();
}

void DcpMaster::initialize() {
    slavesWaitingForInitialize++;
    if(slavesWaitingForInitialize < slaveDescriptions.size()) {
        return;
    }
    std::cout << "Initialize Slaves" << std::endl;
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        manager->STC_initialize(u_char(i), DcpState::CONFIGURED);
    }
    intializationRuns++;
}

void DcpMaster::configuration() {
    slavesWaitingForConfiguration++;
    if(slavesWaitingForConfiguration < slaveDescriptions.size()) {
        return;
    }

    std::cout << "Configure Slaves" << std::endl;

    //Count received acks so that we know when all configuration messages are acknowledged
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        receivedAcks[dcpId_t(i+1)] = 0;
        numOfCmd[dcpId_t(i+1)] = 0;
    }

    for(size_t i=0; i<connections.size(); ++i) {
        std::cout << "connection " << i << "...\n";
        uint16_t dataId = uint16_t(i+1);
        uint8_t fromSlaveId = uint8_t(connections[i].fromSlave);
        manager->CFG_scope(fromSlaveId, dataId, DcpScope::Initialization_Run_NonRealTime);
        manager->CFG_output(fromSlaveId, dataId, 0, connections[i].fromVr);
        manager->CFG_steps(fromSlaveId, dataId, 1);
        numOfCmd[fromSlaveId] += 4;

        for(size_t j=0; j<connections[i].toSlaves.size(); ++j) {
            uint8_t toSlaveId = uint8_t(connections[i].toSlaves[j]);
            manager->CFG_scope(toSlaveId, dataId, DcpScope::Initialization_Run_NonRealTime);
            manager->CFG_input(toSlaveId, dataId, 0, connections[i].toVrs[j], DcpDataType::float64);
            manager->CFG_steps(toSlaveId, dataId, 1);
            manager->CFG_source_network_information_UDP(toSlaveId, dataId, asio::ip::address_v4::from_string(*slaveDescriptions[toSlaveId-1]->TransportProtocols.UDP_IPv4->Control->host).to_uint(), *slaveDescriptions[toSlaveId-1]->TransportProtocols.UDP_IPv4->Control->port+1);
            manager->CFG_target_network_information_UDP(fromSlaveId, dataId,  asio::ip::address_v4::from_string(*slaveDescriptions[toSlaveId-1]->TransportProtocols.UDP_IPv4->Control->host).to_uint(), *slaveDescriptions[toSlaveId-1]->TransportProtocols.UDP_IPv4->Control->port);
            numOfCmd[toSlaveId] += 4;
        }
    }
}

void DcpMaster::configure() {
    slavesWaitingForConfigure++;
    if(slavesWaitingForConfigure < slaveDescriptions.size()) {
        return;
    }
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        manager->STC_configure(u_char(i+1), DcpState::PREPARED);
    }
}

void DcpMaster::run(DcpState currentState, uint8_t sender) {
    std::cout << "Run Simulation" << std::endl;
    std::time_t now = std::time(nullptr);
    manager->STC_run(sender, currentState, now + 2);
}

void DcpMaster::doStep() {
    slavesWaitingForStep++;
    if(slavesWaitingForStep < slaveDescriptions.size()) {
        return;
    }
    slavesWaitingForStep = 0;
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        manager->STC_do_step(u_char(i+1),DcpState::RUNNING,1);
    }
}

void DcpMaster::stop() {
    std::chrono::seconds dura(secondsToSimulate + 2);
    std::this_thread::sleep_for(dura);
    std::cout << "Stop Simulation" << std::endl;
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        manager->STC_stop(u_char(i+1), DcpState::RUNNING);
    }
}

void DcpMaster::deregister() {
    slavesWaitingForDeregister++;
    if(slavesWaitingForDeregister < slaveDescriptions.size()) {
        return;
    }
    std::cout << "Deregister Slave" << std::endl;
    for(size_t i=0; i<slaveDescriptions.size(); ++i) {
        manager->STC_deregister(u_char(i+1), DcpState::STOPPED);
    }
}

void DcpMaster::sendOutputs(DcpState currentState, uint8_t sender) {
    std::cout << "Send Outputs" << std::endl;
    manager->STC_send_outputs(sender, currentState);
}

void DcpMaster::receiveAck(uint8_t sender, uint16_t pduSeqId) {
    (void)pduSeqId;
    std::cout << "receiveAck()\n";
    receivedAcks[sender]++;
    if (receivedAcks[sender] == numOfCmd[sender]) {
        manager->STC_prepare(sender, DcpState::CONFIGURATION);
    }
}

void DcpMaster::receiveNAck(uint8_t sender, uint16_t pduSeqId, DcpError errorCode) {
    (void)sender;
    (void)pduSeqId;
    (void)errorCode;
    std::cerr << "Error in slave configuration." << std::endl;
    std::exit(1);
}



void DcpMaster::dataReceived(uint16_t dataId, size_t length, uint8_t payload[]) {
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
            if(nSteps>10000) {
                stop();
            }
            else {
                ++nSteps;
                doStep();
            }
            break;

        case DcpState::COMPUTED:
            sendOutputs(DcpState::COMPUTED, sender);
            break;

        case DcpState::STOPPED:
            deregister();
            break;

        case DcpState::ALIVE:
            //simulation finished
            slavesWaitingAtExit++;
            if(slavesWaitingAtExit < 2) {
                return;
            }
            std::exit(0);

        case DcpState::PREPARING:
        default:
            break;
    }
}

