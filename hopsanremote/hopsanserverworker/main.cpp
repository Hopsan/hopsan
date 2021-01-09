/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <chrono>
#include <signal.h>
#include <thread>
#include <atomic>
#include <array>

#include "zmq.hpp"

#include "hopsanremotecommon/Messages.h"
#include "hopsanremotecommon/MessageUtilities.h"
#include "hopsanremotecommon/FileAccess.h"
#include "hopsanremotecommon/FileReceiver.hpp"

#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "TicToc.hpp"

#ifdef _WIN32
#include "windows.h"
#ifdef _MSC_VER
using pid_t=int;
#endif
#else
#include <spawn.h>
#include <sys/wait.h>
extern char **environ;
#endif

using namespace hopsan;
using namespace std;

typedef chrono::duration<double> fseconds;

HopsanEssentials gHopsanCore;
bool gHaveLoadedComponentLibraries=false;

typedef struct
{
    string fullName;
    vector< vector<double> > *pData = 0;
    vector< double > *pTimeData = 0;
    size_t dataLength = 0;
    size_t dataId = 0;
    string unit;
    string quantity;
    string alias;
}ModelVariableInfo_t;

string gWorkerId, gUserName;
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
    tmvi.quantity = "Time";
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
                            mvi.alias = pPort->getVariableAlias(pVarDesc->id).c_str();
                            mvi.quantity = pVarDesc->quantity.c_str();
                            mvi.unit = pVarDesc->unit.c_str();
                            mvi.pData = pLogData;
                            mvi.dataId = pVarDesc->id;
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
                        // Set component name and restore the (start value) name
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
                        // Set component name and restore the (start value) name
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
FileReceiver gModelAssets;
std::atomic_bool gIsSimulating(false);
std::atomic_bool gShellIsExecuting(false);
bool gClientConnected = true; //Need to start this as true, to avoid instant quit if client is slow to connect
bool gIsModelLoaded = false;
bool gWasSimulationOK = false;
bool gSimulationFinnished = false;
bool gShellExecExitOK = false;
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
    // status requests received would return incorrect values in such cases
    gSimulationFinnished = false;
    gIsSimulating = true;
    *pSimOK = false;

    // Now launch the simulation thread
    std::thread ( simulationThread, pSimOK ).detach();
}

void waitShellExecThread(pid_t *pPid, std::string *pShellOutput, bool *pExitstatusOK)
{
    gShellIsExecuting = true;
#ifdef _WIN32

#else
    // Wait for process to prevent zombies from taking over the world
    int status=1;
    pid_t waitrc=-1;
    while (waitrc==-1)
    {
        waitrc = waitpid(*pPid, &status, 0);
    }
    *pExitstatusOK = (WIFEXITED(status)!=0) && (WEXITSTATUS(status)==0);

    std::cout << PRINTWORKER << nowDateTime() << " Waitpid on pid: "<< *pPid << " rc: " << waitrc << " WIFEXITED(status): " << WIFEXITED(status) << " status: " << status << " WEXITSTATUS(status): " << WEXITSTATUS(status) <<  endl;
    *pShellOutput = "No output available!";
#endif
    gShellIsExecuting = false;
}



void loadComponentLibraries(const std::string &rDir, bool doRecurse)
{
    FileAccess fa;
    if (fa.enterDir(rDir))
    {
        vector<string> soFiles = fa.findFilesWithSuffix(SHAREDLIB_SUFFIX, doRecurse);
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
    // Load component libraries if not already done
    if (!gHaveLoadedComponentLibraries)
    {
        // Load Hopsan default component library
        loadComponentLibraries("../componentLibraries/defaultLibrary", false);
        // Load common shared libraries
        loadComponentLibraries("./componentLibraries", true);
        // Load user specific libraries
        if (!gUserName.empty())
        {
            loadComponentLibraries("./"+gUserName, true);
        }

        gHaveLoadedComponentLibraries=true;
    }

    // Remember number of errors during loading libraries so that we can detect additional errors below when loading the model
    // Even if some library could not load, the model might still load successfully (if at least one library could be loaded, usually the default library)
    size_t numLibErrors = gHopsanCore.getNumErrorMessages()+gHopsanCore.getNumFatalMessages();
    if (numLibErrors > 0)
    {
        cout << PRINTWORKER << nowDateTime() << " Errors when loading some libraries" << endl;
        gHopsanCore.getCoreMessageHandler()->printMessagesToStdOut();
    }

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

    // Check so that model is loaded ant that no additional error messages were returned
    if (gpRootSystem && (gHopsanCore.getNumErrorMessages()+gHopsanCore.getNumFatalMessages() <= numLibErrors) )
    {
        cout << PRINTWORKER << nowDateTime() << " Model was loaded sucessfully" << endl;
        gIsModelLoaded = true;
        return true;
    }
    else
    {
        cout << PRINTWORKER << nowDateTime() << " Error: Could not load the model" << endl;
        gHopsanCore.getCoreMessageHandler()->printMessagesToStdOut();
        return false;
    }
}


void sendGoodbyToServer(zmq::socket_t &rSocket)
{
    zmq::message_t response;
    sendMessage(rSocket, WorkerFinished, gWorkerId);
    receiveWithTimeout(rSocket, 5000, response); // Wait for but ignore replay
}

void sendAliveToServer(zmq::socket_t &rSocket)
{
    zmq::message_t response;
    sendMessage(rSocket, WorkerAlive, gWorkerId);
    receiveWithTimeout(rSocket, 5000, response); // Wait for but ignore replay
}

string gExecuteInShellOutput;

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

    gUserName="anonymous";
    {
        FileAccess fa;
        fa.enterDir(".");
        fa.createDir(gUserName);
    }
    gModelAssets.setFileDestination("./"+gUserName);

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
        chrono::steady_clock::time_point lastServerReportTime = chrono::steady_clock::now();

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
                cout << PRINTWORKER << nowDateTime() << " Received message with length: " << request.size() << " msg_id: " << msg_id << endl;
                if (msg_id == RequestWorkerStatus)
                {
                    cout << PRINTWORKER << nowDateTime() << " Got status request" << endl;
                    ReplymsgReplyWorkerStatus msg;
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
                    msg.shell_inprogress = gShellIsExecuting;
                    msg.shell_exitok = gShellExecExitOK;

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
                        CmdmsgSetParameter msg = unpackMessage<CmdmsgSetParameter>(request, offset, parseOK);
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

                    // Send parameter value (as string) or nack
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

                            if (hmf.empty())
                            {
                                sendMessage(socket, NotAck, "Server can not load empty model");
                            }
                            else
                            {
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
                        }
                        else
                        {
                            cout << PRINTWORKER << nowDateTime() << " Error: Could not parse model message" << endl;
                            sendMessage(socket, NotAck, "Could not parse model message");
                        }
                    }
                }
                else if (msg_id == SendFile)
                {
                    bool parseOK;
                    CmdmsgSendFile msg = unpackMessage<CmdmsgSendFile>(request, offset, parseOK);
                    cout << PRINTWORKER << nowDateTime() << " Got file chunk: " << msg.filename << " size: " << msg.data.size() << " finalPart: " << msg.islastpart << endl;
                    if(parseOK)
                    {
                        std::string err;
                        bool isok = gModelAssets.addFilePart(msg, err);
                        if (isok)
                        {
                            sendShortMessage(socket, Ack);
                        }
                        else
                        {
                            cout << PRINTWORKER << err << endl;
                            sendMessage(socket, NotAck, "Could not save file chunk");
                        }
                    }
                    else
                    {
                        cout << PRINTWORKER << nowDateTime() << " Error: Could not parse file chunk" << endl;
                        sendMessage(socket, NotAck, "Could not parse file chunk");
                    }
                }
                else if (msg_id == RequestFile)
                {
                    bool parseOK;
                    CmdmsgRequestFile msg = unpackMessage<CmdmsgRequestFile>(request, offset, parseOK);
                    cout << PRINTWORKER << nowDateTime() << " Got file request: " << msg.filename << " offset: " << msg.offset << endl;
                    if(parseOK)
                    {
                        string filepath="./";
                        if (!gUserName.empty())
                        {
                            filepath+=gUserName+"/";
                        }
                        filepath += msg.filename;
                        //! @todo this is almost a duplicate of the code in the client send function, could use common function
                        std::ifstream in(filepath, std::ifstream::ate | std::ifstream::binary);
                        if (in.is_open())
                        {
                            int filesize = in.tellg();
                            in.seekg(0); //Rewind file ptr
                            in.seekg(msg.offset); // Point to offset
                            #define MAXFILECHUNKSIZE 2500000 //(2.5 MB)
                            std::vector<char>  buffer(MAXFILECHUNKSIZE);

                            int readBytesNow, remaningBytes=(filesize-msg.offset);
                            // Handle request of empty file
                            if (remaningBytes == 0)
                            {
                                cout << "Sending empty file: " << filepath << endl;
                                string data;
                                CmdmsgSendFile sendmsg {msg.filename, data, true};
                                sendMessage(socket, SendFile, sendmsg);
                            }
                            // Handle non-empty file
                            else
                            {
                                while( !in.eof() && (remaningBytes != 0) )
                                {
                                    readBytesNow = std::min(MAXFILECHUNKSIZE, remaningBytes);
                                    in.read(buffer.data(), readBytesNow);
                                    std::string data(buffer.data(), readBytesNow);

                                    CmdmsgSendFile sendmsg {msg.filename, data, (readBytesNow < MAXFILECHUNKSIZE)};
                                    remaningBytes -= readBytesNow;
                                    cout << "fileSize: " << filesize << " bytesNow: "<< readBytesNow << " remaningBytes: " << remaningBytes << " isDone: " << (readBytesNow < MAXFILECHUNKSIZE) << " data: " << sendmsg.filename << " " <<  sendmsg.data.size() << " " << sendmsg.islastpart << endl;
                                    sendMessage(socket, SendFile, sendmsg);
                                    break;
                                    //cout << "fileSize: " << filesize << " bytesNow: "<< readBytesNow << " remaningBytes: " << remaningBytes << " isDone: " << (readBytesNow < MAXFILECHUNKSIZE) << " Progress: " << *pProgress << endl;
                                }
                            }
                        }
                        else
                        {
                            cout << PRINTWORKER << nowDateTime() << " Error: Could not open file: " << filepath << endl;
                            sendMessage(socket, NotAck, "Could not open file: "+filepath);

                        }
                        in.close();
                    }
                    else
                    {
                        cout << PRINTWORKER << nowDateTime() << " Error: Could not parse file request" << endl;
                        sendMessage(socket, NotAck, "Could not parse file request");
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
                        CmdmsgSimulate msg = unpackMessage<CmdmsgSimulate>(request, offset, parseOK);
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
                else if (msg_id == ExecuteInShell)
                {
                    bool parseOK;
                    string command = unpackMessage<string>(request, offset, parseOK);
                    if (gShellIsExecuting)
                    {
                        std::cout << PRINTWORKER << nowDateTime() << " Error: Failed to Executed command Shell execution is already in progress! " << command << endl;
                        sendMessage(socket, NotAck, "Failed to execute command, execution is already in progress!");
                    }
                    else if (parseOK)
                    {
                        gExecuteInShellOutput.clear();
                        gShellExecExitOK = false;
#ifdef _WIN32
                        gExecuteInShellOutput = "ExecuteInShell not supported on Windows Yet";
                        sendMessage(socket, NotAck, gExecuteInShellOutput);
#else
                        //! @todo This should be a help function shared between server and worker
                        //! @todo how to abort if child hangs
                        //!

                        cout << "Command: " << command << endl;
                        string process = "/bin/bash";
                        string carg = "-c";
                        string command2;
                        if (!gUserName.empty())
                        {
                            command2.append("PATH=`pwd`:$PATH$; cd "+gUserName+"; ");
                        }
                        command2.append(command);

                        std::array<char*, 4> argv = {&process[0], &carg[0], &command2[0], nullptr};
                        for (size_t i=0; i<argv.size()-1; ++i)
                        {
                            cout << argv[i] << " ";
                        }
                        cout << endl;

                        pid_t pid;
                        int status = posix_spawn(&pid,argv.front(),nullptr,nullptr,argv.data(),environ);
                        if(status == 0)
                        {
                            std::cout << PRINTWORKER << nowDateTime() << " Executed command: " << command2 << " with pid: "<< pid << endl;
                            sendShortMessage(socket, Ack);
                        }
                        else
                        {
                            std::cout << PRINTWORKER << nowDateTime() << " Error: Failed to Executed command! " << command << endl;
                            sendMessage(socket, NotAck, "Failed to execute command!");
                        }

                        // Wait for process to prevent zombies from taking over the world
                        gShellIsExecuting = true; // Hmm seems like we need to set this here, since it may take some time for the thread to actually start, and a request may come in between
                        std::thread(waitShellExecThread, &pid, &gExecuteInShellOutput, &gShellExecExitOK).detach();
#endif
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
                        CmdmsgBenchmark benchreq = unpackMessage<CmdmsgBenchmark>(request, offset, parseOK);
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
                    cout << PRINTWORKER << nowDateTime() << " Got Benchmark results request" << endl;
                    //! @todo  Wait for benchmark to complete maybe
                    //!
                    ReplymsgReplyBenchmarkResults msg;
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
                        vector<ReplymsgResultsVariable> vars;
                        for (size_t mvi=0; mvi<vMVI.size(); ++mvi )
                        {
                            const ModelVariableInfo_t &rMvi = vMVI[mvi];

                            vars.push_back(ReplymsgResultsVariable());
                            vars.back().name = rMvi.fullName;
                            vars.back().alias = rMvi.alias;
                            vars.back().quantity = rMvi.quantity;
                            vars.back().unit = rMvi.unit.c_str();
                            vars.back().data.reserve(rMvi.dataLength);
                            // Copy if a data variable
                            if (rMvi.pData)
                            {
                                for (size_t t=0; t<rMvi.dataLength; ++t)
                                {
                                    vars.back().data.push_back((*rMvi.pData)[t][rMvi.dataId]);
                                }
                            }
                            // Copy if a time data variable
                            else if (rMvi.pTimeData)
                            {
                                for (size_t t=0; t<vMVI[mvi].dataLength; ++t)
                                {
                                    vars.back().data.push_back((*rMvi.pTimeData)[t]);
                                }
                            }
                        }

                        sendMessage(socket,ReplyResults,vars);
                    }
                }
                else if (msg_id == RequestMessages)
                {
                    HopsanCoreMessageHandler *pHandler = gHopsanCore.getCoreMessageHandler();
                    vector<ReplymsgReplyMessage> messages;
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
                else if (msg_id == RequestShellOutput)
                {
                    cout << PRINTWORKER << nowDateTime() << " Client requests shell output!" << endl;
                    sendMessage(socket,ReplyShellOutput,gExecuteInShellOutput);
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
                else if (msg_id == IdentifyUser)
                {
                    bool parseOK;
                    CmdmsgIdentifyUser msg = unpackMessage<CmdmsgIdentifyUser>(request, offset, parseOK);
                    if (parseOK)
                    {
                        cout << PRINTWORKER << nowDateTime() << " Client identifying user: " << msg.username << endl;
                        //! @todo check password
                        bool passwordOK=true;
                        if (passwordOK)
                        {
                            gUserName = msg.username;
                            FileAccess fa;
                            fa.enterDir(".");
                            fa.createDir(msg.username);
                            gModelAssets.setFileDestination("./"+msg.username);
                            sendShortMessage(socket, Ack);
                        }
                        else
                        {
                            sendMessage(socket, NotAck, "Wrong message");
                        }
                    }
                    else
                    {
                        sendMessage(socket, NotAck, "Could not parse user identification");
                    }
                }
                else if (msg_id == ClientClosing)
                {
                    cout << PRINTWORKER << nowDateTime() << " Client said godbye!" << endl;
                    if (gIsSimulating || gShellIsExecuting)
                    {
                        sendMessage(socket, NotAck, "Job still running");
                    }
                    else
                    {
                        sendShortMessage(socket, Ack);
                    }
                    gClientConnected = false;
                }
                else if (!idParseOK)
                {
                    cout << PRINTWORKER << nowDateTime() << " Error: Could not parse message id" << endl;
                }
                else
                {
                    stringstream ss;
                    ss << PRINTWORKER << nowDateTime() << " Warning: Unhandled message id: " << msg_id << endl;
                    cout << ss.str() << endl;
                    sendMessage(socket, NotAck, ss.str());
                }
            }
            else
            {
                // Handle timeout / exception
                nClientTimeouts++;
                if (!gShellIsExecuting)
                {
                    if (double(nClientTimeouts)*double(client_timeout)/60000.0 >= dead_client_timout_min)
                    {
                        // Force quit after some time minutes
                        cout << PRINTWORKER << nowDateTime() << " Client has not sent any message for "<< dead_client_timout_min << " minutes. Terminating worker process!"  << endl;
                        //! @todo This means that you can block the server by starting an infinite job and then just disconnect
                        gClientConnected = false;
                    }
                }
            }

            // Periodically report that we are still alive to server
            fseconds dur = chrono::duration_cast<fseconds>(chrono::steady_clock::now() - lastServerReportTime);
            if (dur.count() > 5*60)
            {
                sendAliveToServer(serverSocket);
                lastServerReportTime = chrono::steady_clock::now();
            }

            // If client have said goodbye and we are no longer simulating or shell executing, then exit
            if (!gClientConnected && !gIsSimulating && !gShellIsExecuting)
            {
                keepRunning = false;
            }

            if (s_interrupted)
            {
                cout << PRINTWORKER << nowDateTime() << " Interrupt signal received, killing worker" << std::endl;
                keepRunning=false;
            }
        }

        // Notify server about our exit
        sendGoodbyToServer(serverSocket);

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
