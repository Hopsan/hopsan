//$Id$

#include <streambuf>
#include <sstream>
#include <fstream>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <iostream>
//#include <thread>
#include <vector>
#include "zmq.hpp"

#include "Messages.h"
#include "PackAndSend.h"
#include "global.h"


#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "TicToc.hpp"

#define DEFAULTCOMPONENTLIB "/home/petno25/svn/hopsan/trunk/componentLibraries/defaultLibrary/" TO_STR(DLL_PREFIX) "defaultComponentLibrary" TO_STR(DEBUG_EXT) TO_STR(DLL_EXT)

using namespace hopsan;
using namespace std;

ServerConfig gServerConfig;
HopsanEssentials gHopsanCore;

typedef struct
{
    string fullName;
    vector< vector<double> > *pData;
    size_t dataLength;
    size_t dataId;
    NodeDataDescription *pDataDescription;
    string unit;
}ModelVariableInfo_t;

// Model utitility
//! @todo vector for vars bad maybe list or map better
void collectAllModelVariables(ComponentSystem *pSys, vector<ModelVariableInfo_t> &rvMVI, HString parentSysNames)
{
    HString sysName = pSys->getName();
    vector<Component *> subComps = pSys->getSubComponents();
    for (size_t c=0; c<subComps.size(); ++c)
    {
        Component *pComp = subComps[c];
        if (pComp->isComponentSystem())
        {
            // Collect results for subsystem
            collectAllModelVariables(static_cast<ComponentSystem*>(pComp), rvMVI, parentSysNames+sysName+"$");
        }
        else
        {
            vector<Port*> ports = pComp->getPortPtrVector();
            for (size_t p=0; p<ports.size(); ++p)
            {
                //cout << "port: " << p << " of: " << ports.size() << endl;
                Port *pPort = ports[p];
                if (pPort->isMultiPort())
                {
                    // Ignore multiports, not possible to determin what we want to log anyway
                    continue;
                }

                //! @todo what about time vector
                vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();

                const vector<NodeDataDescription> *pVars = pPort->getNodeDataDescriptions();
                if (pVars)
                {
                    for (size_t v=0; v<pVars->size(); ++v)
                    {
                        // Only write something if data has been logged (skip ports that are not logged)
                        // We assume that the data vector has been cleared
                        //! @todo check if log on
                        if (pLogData->size() > 0)
                        {
                            const NodeDataDescription *pVarDesc = &(*pVars)[v];
                            ModelVariableInfo_t mvi;
                            mvi.fullName = (parentSysNames+sysName+"$"+pComp->getName()+"#"+pPort->getName()+"#"+pVarDesc->name).c_str();
                            //mvi.pDataDescription = pVarDesc;
                            mvi.unit = pVarDesc->unit.c_str();
                            mvi.pData = pLogData;
                            mvi.dataId = v;
                            mvi.dataLength = pSys->getNumActuallyLoggedSamples();

                            rvMVI.push_back(mvi);
                        }
                    }
                }
            }
        }
    }
}


ComponentSystem *pRootSystem=nullptr;
double simStartTime, simStopTime;

int main(int argc, char* argv[])
{
    //ofstream logfile("/home/petno25/svn/hopsan/trunk/HopsanRemote/bin/logfile");
    if (argc < 2)
    {
        cout << "Error: you must specify what base port to use!" << endl;
        //logfile.close();
        return 1;
    }

    cout << "Server Worker Process Starting with port: " << argv[1] << endl;
    //logfile.flush();

    gHopsanCore.loadExternalComponentLib(DEFAULTCOMPONENTLIB);

    // Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind( makeZMQAddress("*", size_t(atoi(argv[1]))).c_str() );

    bool keepRunning=true;
    while (keepRunning) {
        zmq::message_t request;

        // Wait for next request from client
        socket.recv (&request);
        size_t offset=0;
        size_t msg_id = parseMessageId(static_cast<char*>(request.data()), request.size(), offset);
        cout << "Worker received message with length: " << request.size() << " msg_id: " << msg_id << endl;
        if (msg_id == C_SetParam)
        {
            CM_SetParam_t msg = unpackMessage<CM_SetParam_t>(request, offset);
            cout << "Client want to set parameter " << msg.name << " " << msg.value << endl;

            // Set parameter
            //! @todo

            // Send ack or nack
            sendServerAck(socket);
        }
        else if (msg_id == C_GetParam)
        {
            std::string msg = unpackMessage<std::string>(request, offset);
            cout << "Client want to get parameter " << msg << endl;

            // Get parameter
            //! @todo
            bool isOK=true;

            // Send param value (as string) or nack
            if (isOK)
            {
                sendServerStringMessage(socket, S_GetParam_Reply, "value");
            }
            else
            {
                sendServerNAck(socket, "Could not get parameter");
            }
        }
        else if (msg_id == C_SendingHmf)
        {
            std::string hmf = unpackMessage<std::string>(request, offset);
            cout << "Received hmf with size: " << hmf.size() << endl;
            //cout << hmf << endl;
            int dummy;
            //! @todo loadHMFModel will hang if hmf empty
            pRootSystem = gHopsanCore.loadHMFModel(hmf.c_str(), simStartTime, simStopTime, dummy);
            if (pRootSystem)
            {
                //cout << "Loaded OK Sending server ACK" << endl;
                sendServerAck(socket);
            }
            else
            {
                //cout << "Load failed Sending server NACK" << endl;
                sendServerNAck(socket, "Server could not load model");
            }
        }
        else if (msg_id == C_Simulate)
        {
            CM_Simulate_t msg = unpackMessage<CM_Simulate_t>(request, offset);
            // Start simulation
            SimulationHandler simulator;
            bool irc=false,src=false;
            TicToc timer;
            irc = simulator.initializeSystem(simStartTime, simStopTime, pRootSystem);
            timer.TocPrint("Initialize");
            if (irc)
            {
                timer.Tic();
                src = simulator.simulateSystem(simStartTime, simStopTime, 1, pRootSystem);
                timer.TocPrint("Simulate");
            }
            timer.Tic();
            simulator.finalizeSystem(pRootSystem);
            timer.TocPrint("Finalize");

            if (irc && src)
            {
                sendServerAck(socket);
            }
            else if (!irc)
            {
                cout  << "Model Init failed"  << endl;
                sendServerNAck(socket, "Could not initialize system");
            }
            else
            {
                cout  << "Model simulation failed"  << endl;
                sendServerNAck(socket, "Cold not simulate system");
            }
        }
        else if (msg_id == C_ReqResults)
        {
            string varName = unpackMessage<string>(request, offset);
            cout << "Client requests variable: " << varName << endl;


            vector<ModelVariableInfo_t> vMVI;
            collectAllModelVariables(pRootSystem, vMVI, "");

            cout << "vMVI size: " << vMVI.size() << endl;

            // check if simulation finnished, ACK Nack
            vector<SM_Variable_Description_t> vars;
            //vector<double> dd {1, 2, 3, 4};
            //SM_Variable_Description_t var {"kalle", "apa", "m^2", dd};

            for (size_t mvi=0; mvi<vMVI.size(); ++mvi )
            {
                vars.push_back(SM_Variable_Description_t());
                vars.back().name = vMVI[mvi].fullName.c_str();
                vars.back().alias = "";
                vars.back().unit = vMVI[mvi].unit.c_str();
                vars.back().data.reserve(vMVI[mvi].dataLength);
                for (size_t t=0; t<vMVI[mvi].dataLength; ++t)
                {
                    vars.back().data.push_back((*vMVI[mvi].pData)[t][vMVI[mvi].dataId]);
                }
            }
            //vars.push_back(var);
            //dd = {5, 6, 7, 8};
            //var = {"hona", "", "m/s", dd};
            //vars.push_back(var);

            sendMessage<vector<SM_Variable_Description_t>>(socket,S_ReqResults_Reply,vars);
            //sendServerNAck(socket, "Simulation not finnished yet!");

        }
        else if (msg_id == C_ReqMessages)
        {
            cout << "Client requests messages" << endl;
            HopsanCoreMessageHandler *pHandler = gHopsanCore.getCoreMessageHandler();
            vector<SM_HopsanCoreMessage_t> messages;
            size_t nMessages = pHandler->getNumWaitingMessages();
            messages.resize(nMessages);
            for (size_t i=0; i<nMessages; ++i)
            {
                HString mess, tag, type;
                pHandler->getMessage(mess, type, tag);
                messages[i].message = mess.c_str();
                messages[i].tag = tag.c_str();
                messages[i].type = type[0];
            }

            sendMessage<vector<SM_HopsanCoreMessage_t>>(socket,S_ReqMessages_Reply,messages);
        }
        else if (msg_id == C_Bye)
        {
            cout << "Client said godbye!" << endl;
            sendServerAck(socket);
            keepRunning = false;
        }
        else
        {
            stringstream ss;
            ss << "Server error: Unknown message id " << msg_id << endl;
            cout << "Server error: Unknown message id " << msg_id << endl;
            sendServerNAck(socket, ss.str());
        }

        // Do some 'work'
#ifndef _WIN32
        //sleep(1);
#else
        //Sleep (1);
#endif
        //! @todo need to clear the hopsan model
    }
    //logfile.close();
}
