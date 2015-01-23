//$Id$

//#include <streambuf>
#include <sstream>
//#include <fstream>

//#ifndef _WIN32
//#include <unistd.h>
//#else
//#include <windows.h>
//#endif

#include <iostream>
#include <vector>
#include "zmq.hpp"

#include "Messages.h"
#include "MessageUtilities.h"
//#include "global.h"
#include "FileAccess.h"


#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "TicToc.hpp"

using namespace hopsan;
using namespace std;

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

void loadComponentLibraries()
{
    FileAccess fa;
    fa.enterDir("./componentLibraries");
    vector<string> soFiles = fa.findFilesWithSuffix("so");
    for (string f : soFiles)
    {
        cout << "Loading library file: " << f << endl;
        gHopsanCore.loadExternalComponentLib(f.c_str());
    }
}


ComponentSystem *pRootSystem=nullptr;
double simStartTime, simStopTime;
size_t nThreads = 1;

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << "Error: To few arguments!" << endl;
        return 1;
    }

    //cout << "argv: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
    string workerId = argv[1];
    string serverCtrlPort = argv[2];
    string workerCtrlPort = argv[3];

    // Read num threads argument
    if (argc == 5)
    {
        nThreads = size_t(atoi(argv[4]));
    }

    cout << "Server Worker Process with ID: " << workerId << " Listening on port: " << workerCtrlPort << " Using: " << nThreads << " threads" << endl;

    // Loading component libraries
    loadComponentLibraries();

    // Prepare our context and sockets
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind( makeZMQAddress("*", workerCtrlPort).c_str() );

    zmq::socket_t serverSocket( context, ZMQ_REQ);
    serverSocket.connect( makeZMQAddress("localhost", serverCtrlPort).c_str() );

    bool keepRunning=true;
    while (keepRunning) {
        zmq::message_t request;

        // Wait for next request from client
        socket.recv (&request);
        size_t offset=0;
        size_t msg_id = getMessageId(request, offset);
        //cout << "Worker received message with length: " << request.size() << " msg_id: " << msg_id << endl;
        if (msg_id == C_SetParam)
        {
            CM_SetParam_t msg = unpackMessage<CM_SetParam_t>(request, offset);
            cout << "Client want to set parameter " << msg.name << " " << msg.value << endl;

            // Set parameter
            //! @todo

            // Send ack or nack
            sendServerNAck(socket, "Not implemented");
        }
        else if (msg_id == C_GetParam)
        {
            std::string msg = unpackMessage<std::string>(request, offset);
            cout << "Client want to get parameter " << msg << endl;

            // Get parameter
            //! @todo
            bool isOK=false;

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
            cout << "Received hmf with size: " << hmf.size() << endl; //<< hmf << endl;

            // If a model is already loaded then delete it
            if (pRootSystem)
            {
                delete pRootSystem;
                pRootSystem=nullptr;
            }

            //! @todo loadHMFModel will hang if hmf empty
            pRootSystem = gHopsanCore.loadHMFModel(hmf.c_str(), simStartTime, simStopTime);
            if (pRootSystem)
            {
                sendServerAck(socket);
            }
            else
            {
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
            vector<ModelVariableInfo_t> vMVI;
            collectAllModelVariables(pRootSystem, vMVI, "");
            cout << "Client requests variable: " << varName << " Sending: " << vMVI.size() << " variables!" << endl;

            //! @todo Check if simulation finnished, ACK Nack
            vector<SM_Variable_Description_t> vars;
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

            sendServerMessage<vector<SM_Variable_Description_t>>(socket,S_ReqResults_Reply,vars);
        }
        else if (msg_id == C_ReqMessages)
        {
            HopsanCoreMessageHandler *pHandler = gHopsanCore.getCoreMessageHandler();
            vector<SM_HopsanCoreMessage_t> messages;
            size_t nMessages = pHandler->getNumWaitingMessages();
            messages.resize(nMessages);
            cout << "Client requests messages! " <<  "Sending: " << nMessages << " messages!" << endl;
            for (size_t i=0; i<nMessages; ++i)
            {
                HString mess, tag, type;
                pHandler->getMessage(mess, type, tag);
                messages[i].message = mess.c_str();
                messages[i].tag = tag.c_str();
                messages[i].type = type[0];
            }

            sendServerMessage<vector<SM_HopsanCoreMessage_t>>(socket,S_ReqMessages_Reply,messages);
        }
        else if (msg_id == C_Bye)
        {
            cout << "Client said godbye!" << endl;
            sendServerAck(socket);
            keepRunning = false;

            sendServerStringMessage(serverSocket, SW_Finished, argv[1]);
            serverSocket.recv(&request); // Wait for but ignore replay
        }
        else
        {
            stringstream ss;
            ss << "Server error: Unknown message id: " << msg_id << endl;
            cout << ss.str() << endl;
            sendServerNAck(socket, ss.str());
        }

        //! @todo do we need to sleep here?
#ifndef _WIN32
        //sleep(1);
#else
        //Sleep (1);
#endif
    }

    // Delete the model if we have one
    if (pRootSystem)
    {
        delete pRootSystem;
        pRootSystem=nullptr;
    }


}
