//$Id$

#include <sstream>
#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <signal.h>
#include <thread>
#include <atomic>

#include "zmq.hpp"

#include "Messages.h"
#include "MessageUtilities.h"
#include "FileAccess.h"

#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "TicToc.hpp"

#ifdef _WIN32
#include "windows.h"
#endif

using namespace hopsan;
using namespace std;

HopsanEssentials gHopsanCore;

typedef struct
{
    string fullName;
    vector< vector<double> > *pData = 0;
    vector< double > *pTimeData = 0;
    size_t dataLength = 0;
    size_t dataId = 0;
    NodeDataDescription *pDataDescription = 0;
    string unit;
}ModelVariableInfo_t;

string gWorkerId;
#define PRINTWORKER "Worker "+gWorkerId+"; "

std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return std::string(&buff[0]);
}

// ------------------------------
// Model utilities BEGIN
// ------------------------------
//! @todo Model utilities should not be in this file they should be shared

//! @todo vector for vars bad maybe list or map better
void collectAllModelVariables(ComponentSystem *pSys, vector<ModelVariableInfo_t> &rvMVI, HString systemHierarchy)
{
    // Append this systems time vector
    vector<double> *pTime = pSys->getLogTimeVector();
    ModelVariableInfo_t tmvi;
    tmvi.fullName = (systemHierarchy+"Time").c_str();
    tmvi.unit = "s";
    tmvi.pTimeData = pTime;
    tmvi.dataLength = pSys->getNumActuallyLoggedSamples();
    rvMVI.push_back(tmvi);

    vector<Component *> subComps = pSys->getSubComponents();
    for (size_t c=0; c<subComps.size(); ++c)
    {
        Component *pComp = subComps[c];
        if (pComp->isComponentSystem())
        {
            // Collect results for subsystem
            collectAllModelVariables(static_cast<ComponentSystem*>(pComp), rvMVI, systemHierarchy+pComp->getName()+"$");
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
                    // Ignore multiports, not possible to determine what we want to log anyway
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
                            mvi.fullName = (systemHierarchy+pComp->getName()+"#"+pPort->getName()+"#"+pVarDesc->name).c_str();
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

void splitStringOnDelimiter(const std::string &rString, const char delim, std::vector<std::string> &rSplitVector)
{
    rSplitVector.clear();
    string item;
    stringstream ss(rString);
    while(getline(ss, item, delim))
    {
        rSplitVector.push_back(item);
    }
}

bool setParameter(ComponentSystem *pSystem, HString &fullname, const HString &rValue)
{
    if (pSystem)
    {
        size_t d = fullname.find_first_of('$');
        if (d != HString::npos)
        {
            HString sysname = fullname.substr(0, d);
            fullname.erase(0, d+1);
            ComponentSystem *pSubsys = pSystem->getSubComponentSystem(sysname);
            return setParameter(pSubsys, fullname, rValue);
        }
        else
        {
            //! @todo write split function in hstring class
            string item;
            stringstream ss(fullname.c_str());
            vector<string> cpv;
            while(getline(ss, item, '#'))
            {
                cpv.push_back(item);
            }

            if (cpv.size() == 2 || cpv.size() == 3)
            {
                Component *pComp = pSystem->getSubComponent(cpv[0].c_str());
                if (pComp)
                {
                    string parameter;
                    if (cpv.size() == 2)
                    {
                        parameter = cpv[1];
                    }
                    else if (cpv.size() == 3)
                    {
                        // Set component name and restor the (startvalue) name
                        parameter = cpv[1]+"#"+cpv[2];
                    }

                    return pComp->setParameterValue(parameter.c_str(), rValue);
                }
            }
        }
    }
    return false;
}

string getParameter(ComponentSystem *pSystem, HString &fullname)
{
    if (pSystem)
    {
        size_t d = fullname.find_first_of('$');
        if (d != HString::npos)
        {
            HString sysname = fullname.substr(0, d);
            fullname.erase(0, d+1);
            ComponentSystem *pSubsys = pSystem->getSubComponentSystem(sysname);
            return getParameter(pSubsys, fullname);
        }
        else
        {
            //! @todo write split function in hstring class
            string item;
            stringstream ss(fullname.c_str());
            vector<string> cpv;
            while(getline(ss, item, '#'))
            {
                cpv.push_back(item);
            }

            if (cpv.size() == 2 || cpv.size() == 3)
            {
                Component *pComp = pSystem->getSubComponent(cpv[0].c_str());
                if (pComp)
                {
                    string parameter;
                    if (cpv.size() == 2)
                    {
                        parameter = cpv[1];
                    }
                    else if (cpv.size() == 3)
                    {
                        // Set component name and restor the (startvalue) name
                        parameter = cpv[1]+"#"+cpv[2];
                    }

                    HString value;
                    pComp->getParameterValue(parameter.c_str(), value);
                    return value.c_str();
                }
            }
        }
    }
    return "";
}

// ------------------------------
// Model utilities END
// ------------------------------


ComponentSystem *gpRootSystem=nullptr;
double gSimStartTime, gSimStopTime;
size_t gNumThreads = 1;
SimulationHandler gSimulator;
std::atomic_bool gIsSimulating(false);
bool gIsModelLoaded = false;
bool gWasSimulationOK = false;
bool gSimulationFinnished = false;
double gInitTime;
double gSimulationTime;
double gFinilizeTime;

static int s_interrupted = 0;
#ifdef _WIN32
BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
{
    // what to do here?
    s_interrupted = 1;
    return TRUE;
}
#else
static void s_signal_handler(int signal_value)
{
    s_interrupted = 1;
}

static void s_catch_signals(void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}
#endif


void simulationThread(bool *pSimOK)
{
    TicToc timer;

    timer.Tic();
    bool simOK = gSimulator.simulateSystem(gSimStartTime, gSimStopTime, gNumThreads, gpRootSystem);
    gSimulationTime = timer.TocPrint(PRINTWORKER+nowDateTime()+" Simulate");

    timer.Tic();
    gSimulator.finalizeSystem(gpRootSystem);
    gFinilizeTime = timer.TocPrint(PRINTWORKER+nowDateTime()+" Finalize");

    // We need to set this last, as it is used to "signal" simulation complete
    *pSimOK = simOK;
    gIsSimulating = false;
    gSimulationFinnished = true;
}

void startSimulation(bool *pSimOK)
{
    // We set this first to signal that simulation is not yet finished
    // we need to set them before launching the thread, since it may take a while to start it
    // staus requests received would return incorrect values in such cases
    gSimulationFinnished = false;
    gIsSimulating = true;
    *pSimOK = false;

    // Now launch the simulation thread
    std::thread ( simulationThread, pSimOK ).detach();
}



void loadComponentLibraries(const std::string &rDir)
{
    FileAccess fa;
    if (fa.enterDir(rDir))
    {
        vector<string> soFiles = fa.findFilesWithSuffix(TO_STR(DLL_EXT));
        for (string f : soFiles)
        {
            cout << PRINTWORKER << nowDateTime() << " Loading library file: " << f << endl;
            gHopsanCore.loadExternalComponentLib(f.c_str());
        }
    }
    else
    {
        cout << PRINTWORKER << nowDateTime() << " Error: Could not enter directory: " << rDir << endl;
    }
}

bool loadModel(string &rModel)
{
    // If a model is already loaded then delete it
    if (gpRootSystem)
    {
        delete gpRootSystem;
        gpRootSystem=nullptr;
        gIsModelLoaded = false;
    }

    //! @todo loadHMFModel will hang (sometimes) if hmf empty
    if (!rModel.empty())
    {
        gpRootSystem = gHopsanCore.loadHMFModel(rModel.c_str(), gSimStartTime, gSimStopTime);
    }

    if (gpRootSystem && (gHopsanCore.getNumErrorMessages() == 0) && (gHopsanCore.getNumFatalMessages() == 0) )
    {
        cout << PRINTWORKER << nowDateTime() << " Model was loaded sucessfully" << endl;
        gIsModelLoaded = true;
        return true;
    }
    else
    {
        cout << PRINTWORKER << nowDateTime() << " Error: Could not load the model" << endl;
        return false;
    }
}


void sendServerGoodby(zmq::socket_t &rSocket)
{
    zmq::message_t response;
    sendMessage(rSocket, Finished, gWorkerId);
    receiveWithTimeout(rSocket, 5000, response); // Wait for but ignore replay
}


int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << PRINTWORKER << nowDateTime() << " Error: To few arguments!" << endl;
        return 1;
    }

    //cout << "argv: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
    gWorkerId = argv[1];
    string serverCtrlPort = argv[2];
    string workerCtrlPort = argv[3];

    // Read num threads argument
    if (argc == 5)
    {
        gNumThreads = size_t(atoi(argv[4]));
    }

    cout << PRINTWORKER << nowDateTime() << " Listening on port: " << workerCtrlPort << " Using: " << gNumThreads << " threads" << endl;
    cout << PRINTWORKER << nowDateTime() << " Server control port is: " << serverCtrlPort << endl;

    // Loading component libraries
    loadComponentLibraries("./componentLibraries");
    loadComponentLibraries("../componentLibraries/defaultLibrary");

    // Prepare our context and sockets
    try
    {
#ifdef _WIN32
        zmq::context_t context(1, 63);
#else
        zmq::context_t context(1);
#endif

        zmq::socket_t socket (context, ZMQ_REP);
        zmq::socket_t serverSocket( context, ZMQ_REQ);

        int linger_ms = 1000;
        socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
        serverSocket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));

        socket.bind( makeZMQAddress("*", workerCtrlPort).c_str() );
        serverSocket.connect( makeZMQAddress("localhost", serverCtrlPort).c_str() );

        const long client_timeout = 30000;
        const double dead_client_timout_min = 5;
        size_t nClientTimeouts = 0;

#ifdef _WIN32
        SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
#else
        s_catch_signals();
#endif
        bool keepRunning=true;
        while (keepRunning)
        {
            // Wait for next request from client
            zmq::message_t request;
            if(receiveWithTimeout(socket, client_timeout, request))
            {
                nClientTimeouts = 0;
                size_t offset=0;
                bool idParseOK;
                size_t msg_id = getMessageId(request, offset, idParseOK);
                //cout << PRINTWORKER << nowDateTime() << " Received message with length: " << request.size() << " msg_id: " << msg_id << endl;
                if (msg_id == RequestWorkerStatus)
                {
                    cout << PRINTWORKER << nowDateTime() << " Got status request" << endl;
                    replymsg_ReplyWorkerStatus_t msg;
                    msg.model_loaded = gIsModelLoaded;
                    msg.simualtion_success = gWasSimulationOK;
                    msg.simulation_finished = gSimulationFinnished;
                    msg.simulation_inprogress = gIsSimulating;
                    if (msg.simulation_inprogress || msg.simulation_finished)
                    {
                        msg.current_simulation_time = gpRootSystem->getTime();
                        msg.simulation_progress = (msg.current_simulation_time-gSimStartTime) / (gSimStopTime - gSimStartTime);
                        msg.estimated_simulation_time_remaining = -1;
                    }
                    else
                    {
                        msg.current_simulation_time = -1;
                        msg.simulation_progress = -1;
                        msg.estimated_simulation_time_remaining = -1;
                    }

                    sendMessage(socket, ReplyWorkerStatus, msg);
                }
                else if (msg_id == SetParameter)
                {
                    if (gIsSimulating)
                    {
                        sendMessage(socket, NotAck, "You can not set parameters while simulating!");
                    }
                    else
                    {
                        bool parseOK;
                        cmdmsg_SetParameter_t msg = unpackMessage<cmdmsg_SetParameter_t>(request, offset, parseOK);
                        cout << PRINTWORKER << nowDateTime() << " Client want to set parameter " << msg.name << " " << msg.value << endl;

                        // Set parameter
                        HString fullName = msg.name.c_str();
                        bool rc = setParameter(gpRootSystem, fullName, msg.value.c_str());
                        // Send ack or nack
                        if (rc)
                        {
                            sendShortMessage(socket, Ack);
                        }
                        else
                        {
                            sendMessage(socket, NotAck, "Failed to set parameter: "+msg.name);
                        }
                    }
                }
                else if (msg_id == RequestParameter)
                {
                    //! @todo is this safe while simulating

                    bool parseOK;
                    std::string msg = unpackMessage<std::string>(request, offset, parseOK);
                    cout << PRINTWORKER << nowDateTime() << " Client want to get parameter " << msg << endl;

                    // Get parameter
                    HString fullName = msg.c_str();
                    string val = getParameter(gpRootSystem, fullName);
                    //! @todo what if root system name is first?

                    // Send param value (as string) or nack
                    if (val.empty())
                    {
                        sendMessage(socket, NotAck, "Could not get parameter");
                    }
                    else
                    {
                        sendMessage(socket, ReplyParameter, val);
                    }
                }
                else if (msg_id == SetModel)
                {
                    if (gIsSimulating)
                    {
                        sendMessage(socket, NotAck, "You can not load a model while simulating!");
                    }
                    else
                    {
                        bool parseOK;
                        std::string hmf = unpackMessage<std::string>(request, offset, parseOK);
                        if (parseOK)
                        {
                            cout << PRINTWORKER << nowDateTime() << " Received hmf with size: " << hmf.size() << endl; //<< hmf << endl;

                            bool rc = loadModel(hmf);
                            if (rc )
                            {
                                sendShortMessage(socket, Ack);
                            }
                            else
                            {
                                sendMessage(socket, NotAck, "Server could not load model");
                            }
                        }
                        else
                        {
                            cout << PRINTWORKER << nowDateTime() << " Error: Could not parse model message" << endl;
                            sendMessage(socket, NotAck, "Could not parse model message");
                        }
                    }
                }
                else if (msg_id == Simulate)
                {
                    if (gIsSimulating)
                    {
                        sendMessage(socket, NotAck, "Simulation is already in progress!");
                    }
                    else
                    {
                        bool parseOK;
                        cmdmsg_Simulate_t msg = unpackMessage<cmdmsg_Simulate_t>(request, offset, parseOK);
                        if (parseOK)
                        {
                            // Start simulation
                            bool irc=false;
                            TicToc timer;
                            //! @todo what happens here if we get stuck in initialize, then get status will say that we are not simulating
                            irc = gSimulator.initializeSystem(gSimStartTime, gSimStopTime, gpRootSystem);
                            gInitTime = timer.TocPrint(PRINTWORKER+nowDateTime()+" Initialize");
                            if (irc)
                            {
                                //std::thread ( simulationThread, &gWasSimulationOK ).detach();
                                startSimulation(&gWasSimulationOK);
                                sendShortMessage(socket, Ack);
                            }
                            else
                            {
                                cout  << PRINTWORKER << nowDateTime() << " Model Init failed"  << endl;
                                sendMessage(socket, NotAck, "Could not initialize system");
                            }
                        }
                        else
                        {
                            cout  << PRINTWORKER << nowDateTime() << " Error: Failed to parse simulation message" << endl;
                            sendMessage(socket, NotAck, "Failed to parse simulation message");
                        }
                    }
                }
                else if (msg_id == Benchmark)
                {
                    if (gIsSimulating)
                    {
                        sendMessage(socket, NotAck, "Simulation is already in progress!");
                    }
                    else
                    {
                        bool parseOK;
                        cmdmsg_Benchmark_t benchreq = unpackMessage<cmdmsg_Benchmark_t>(request, offset, parseOK);
                        if (parseOK)
                        {
                            bool rc = loadModel(benchreq.model);
                            if (rc )
                            {

                                TicToc timer;
                                bool irc = gSimulator.initializeSystem(gSimStartTime, gSimStopTime, gpRootSystem);
                                gInitTime = timer.TocPrint(PRINTWORKER+nowDateTime()+" Initialize");
                                if (irc)
                                {
                                    // Start simulation
                                    //std::thread ( simulationThreads, &gWasSimulationOK ).detach();
                                    startSimulation(&gWasSimulationOK);
                                    sendShortMessage(socket, Ack);
                                }
                                else
                                {
                                    cout  << PRINTWORKER << nowDateTime() << " Model Init failed"  << endl;
                                    sendMessage(socket, NotAck, "Could not initialize system");
                                }
                            }
                            else
                            {
                                sendMessage(socket, NotAck, "Server could not load the model to benchmark");
                            }
                        }
                        else
                        {
                            cout  << PRINTWORKER << nowDateTime() << " Error: Failed to parse simulation message" << endl;
                            sendMessage(socket, NotAck, "Failed to parse simulation message");
                        }
                    }
                }
                else if (msg_id == RequestBenchmarkResults)
                {
                    cout << PRINTWORKER << nowDateTime() << " Got Benchmark times request" << endl;
                    //! @todo  Wait for benchmark to complete maybe
                    //!
                    replymsg_ReplyBenchmarkResults_t msg;
                    msg.numthreads = gNumThreads;
                    msg.inittime = gInitTime;
                    msg.simutime = gSimulationTime;
                    msg.finitime = gFinilizeTime;

                    sendMessage(socket, ReplyBenchmarkResults, msg);
                }

                else if (msg_id == RequestResults)
                {
                    if (gIsSimulating)
                    {
                        sendMessage(socket, NotAck, "Simulation is still in progress!");
                    }
                    else
                    {
                        bool parseOK;
                        string varName = unpackMessage<string>(request, offset, parseOK);
                        vector<ModelVariableInfo_t> vMVI;
                        collectAllModelVariables(gpRootSystem, vMVI, "");
                        cout << PRINTWORKER << nowDateTime() << " Client requests variable: " << varName << " Sending: " << vMVI.size() << " variables!" << endl;

                        //! @todo Check if simulation finished, ACK Nack
                        vector<replymsg_ResultsVariable_t> vars;
                        for (size_t mvi=0; mvi<vMVI.size(); ++mvi )
                        {
                            vars.push_back(replymsg_ResultsVariable_t());
                            vars.back().name = vMVI[mvi].fullName.c_str();
                            vars.back().alias = "";
                            vars.back().unit = vMVI[mvi].unit.c_str();
                            vars.back().data.reserve(vMVI[mvi].dataLength);
                            // Copy if a data variable
                            if (vMVI[mvi].pData)
                            {
                                for (size_t t=0; t<vMVI[mvi].dataLength; ++t)
                                {
                                    vars.back().data.push_back((*vMVI[mvi].pData)[t][vMVI[mvi].dataId]);
                                }
                            }
                            // Copy if a time data variable
                            else if (vMVI[mvi].pTimeData)
                            {
                                for (size_t t=0; t<vMVI[mvi].dataLength; ++t)
                                {
                                    vars.back().data.push_back((*vMVI[mvi].pTimeData)[t]);
                                }
                            }
                        }

                        sendMessage(socket,ReplyResults,vars);
                    }
                }
                else if (msg_id == RequestMessages)
                {
                    HopsanCoreMessageHandler *pHandler = gHopsanCore.getCoreMessageHandler();
                    vector<replymsg_ReplyMessage_t> messages;
                    size_t nMessages = pHandler->getNumWaitingMessages();
                    messages.resize(nMessages);
                    cout << PRINTWORKER << nowDateTime() << " Client requests messages! " <<  "Sending: " << nMessages << " messages!" << endl;
                    for (size_t i=0; i<nMessages; ++i)
                    {
                        HString mess, tag, type;
                        pHandler->getMessage(mess, type, tag);
                        messages[i].message = mess.c_str();
                        messages[i].tag = tag.c_str();
                        messages[i].type = type[0];
                    }

                    sendMessage(socket,ReplyMessages,messages);
                }
                else if (msg_id == Abort)
                {
                    if (gIsSimulating && gpRootSystem)
                    {
                        cout << PRINTWORKER << nowDateTime() << " Client request Abort simulation!" << endl;
                        gpRootSystem->stopSimulation("Got abort request");
                        sendShortMessage(socket, Ack);
                    }
                    else
                    {
                        sendMessage(socket, NotAck, "No simulation running");
                    }
                }
                else if (msg_id == Closing)
                {
                    cout << PRINTWORKER << nowDateTime() << " Client said godbye!" << endl;
                    sendShortMessage(socket, Ack);
                    keepRunning = false;

                    sendServerGoodby(serverSocket);

                    //! @todo what happen to simulation thread if it is running
                }
                else if (!idParseOK)
                {
                    cout << PRINTWORKER << nowDateTime() << " Error: Could not parse message id" << endl;
                }
                else
                {
                    stringstream ss;
                    ss << PRINTWORKER << nowDateTime() << " Error: Unknown message id: " << msg_id << endl;
                    cout << ss.str() << endl;
                    sendMessage(socket, NotAck, ss.str());
                }
            }
            else
            {
                //Handle timout / exception
                nClientTimeouts++;
                if (double(nClientTimeouts)*double(client_timeout)/60000.0 >= dead_client_timout_min)
                {
                    // Force quit after 5 minutes
                    cout << PRINTWORKER << nowDateTime() << " Client has not sent any message for "<< dead_client_timout_min << " minutes. Terminating worker process!"  << endl;
                    //! @todo should hanlde loong simulation time
                    sendServerGoodby(serverSocket);
                    keepRunning = false;
                }
            }

            //! @todo do we need to sleep here?
#ifndef _WIN32
            //sleep(1);
#else
            //Sleep (1);
#endif
            if (s_interrupted)
            {
                cout << PRINTWORKER << nowDateTime() << " Interrupt signal received, killing worker" << std::endl;
                keepRunning=false;
            }
        }

        // Delete the model if we have one
        if (gpRootSystem)
        {
            delete gpRootSystem;
            gpRootSystem=nullptr;
            gIsModelLoaded = false;
        }
    }
    catch(zmq::error_t e)
    {
        cout << PRINTWORKER << nowDateTime() << " Error: Preparing our context and socket: " << e.what() << endl;
    }

    cout << PRINTWORKER << nowDateTime() << " Closed!" << endl;
}
