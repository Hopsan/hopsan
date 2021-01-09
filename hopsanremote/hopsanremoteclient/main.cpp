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

#include "hopsanremoteclient/RemoteHopsanClient.h"

#include <tclap/CmdLine.h>
#include <zmq.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <signal.h>
using namespace std;

static int s_interrupted = 0;
#ifdef _WIN32
#include <windows.h>

BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
{
    // what to do here?
    s_interrupted = 1;
    return TRUE;
}
#else
#include <unistd.h>
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

#define PRINTCLIENT "Client; "

int main(int argc, char* argv[])
{
    try {
        TCLAP::CmdLine cmd("RemoteHopsanClient");

        // Define a value argument and add it to the command line.
        TCLAP::ValueArg<int> longTOOption("","longtimeout","The receive timeout for time-consuming requests",false,30,"Time in seconds (default 30)", cmd);
        TCLAP::ValueArg<int> shortTOOption("","shorttimeout","The receive timeout for small (fast) requests",false,5,"Time in seconds (default 5)", cmd);
        TCLAP::ValueArg<int> numSlotsOption("","numslots", "The number of slots to request",false,1,"Integer (default 1)",cmd);
        TCLAP::SwitchArg nonBlockingShell("", "nonblockingshell", "Don't wait for shell script to finish", cmd);
        TCLAP::MultiArg<std::string> shellOptions("", "shellexec", "Command to execute in shell", false, "string", cmd);
        TCLAP::MultiArg<std::string> requestOptions("", "request", "Request file (only from WD)", false, "string", cmd);
        TCLAP::MultiArg<std::string> assetsOptions("a", "asset", "Model assets (files)", false, "string (filepath)", cmd);
        TCLAP::ValueArg<std::string> userOption("u","user","The user identification string",false,"","user:password or user", cmd);
        TCLAP::ValueArg<std::string> hmfPathOption("m","hmf","The Hopsan model file to load",false,"","Path to file", cmd);
        TCLAP::ValueArg<std::string> serverAddrOption("s", "serverip", "Server IP address and port (default is localhost:45050)", false, "localhost:45050", "IP address", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Prepare our context and socket
#ifdef _WIN32
        zmq::context_t context(1, 63);
#else
        zmq::context_t context(1);
#endif
        RemoteHopsanClient rhopsan(context);
        // Set timeouts
        if (shortTOOption.isSet())
        {
            rhopsan.setShortReceiveTimeout(shortTOOption.getValue()*1000);
        }
        if (longTOOption.isSet())
        {
            rhopsan.setLongReceiveTimeout(longTOOption.getValue()*1000);
        }

        cout << PRINTCLIENT << "Connecting to: " << serverAddrOption.getValue() << endl;
        rhopsan.connectToServer(serverAddrOption.getValue());
        cout << PRINTCLIENT << "Connected: " << rhopsan.serverConnected() << endl;

        string username, password;
        if (userOption.isSet())
        {
            string nameandpasswd = userOption.getValue();
            username = nameandpasswd;
            size_t e = nameandpasswd.find_last_of(':');
            if (e != string::npos)
            {
                username = nameandpasswd.substr(0, e);
                password = nameandpasswd.substr(e+1);
            }
        }

        bool completedOK = true;
        try
        {
            int workerPort;
            bool rc = rhopsan.requestSlot(numSlotsOption.getValue(), workerPort, username);
            if (rc)
            {
                cout << PRINTCLIENT << "Got server worker slot at port: " << workerPort << endl;
                rhopsan.connectToWorker(workerPort);

                // Send user identification
                if (userOption.isSet())
                {
                    rhopsan.sendUserIdentification(username, password);
                }

                // Send model assets
                const std::vector<std::string> &rAssets = assetsOptions.getValue();
                for (const string &rAsset : rAssets)
                {
                    cout << PRINTCLIENT << "Sending asset: " << rAsset <<  " ... ";
                    double progress;
                    // Set relative path to filename only
#ifdef _WIN32
                    //! @todo use common utility (see fileacces in worker)
                    size_t e = rAsset.find_last_of('\\');
                    if (e == string::npos)
                    {
                        e = rAsset.find_last_of('/');
                    }
#else
                    size_t e = rAsset.find_last_of('/');
#endif
                    string relname = rAsset;
                    if (e != string::npos)
                    {
                        relname = rAsset.substr(e+1);
                    }
                    rhopsan.blockingSendFile(rAsset, relname, &progress);
                    cout << "Done!" << endl;
                }

                // If model is set then try to open it and simulate it remotely
                bool simulationOK=true;
                if (hmfPathOption.isSet())
                {
                    ifstream hmf_file(hmfPathOption.getValue());
                    if (!hmf_file.is_open())
                    {
                        cout << PRINTCLIENT << "Error: Could not open model file " << hmfPathOption.getValue() << endl;
                        simulationOK=false;
                    }


                    std::stringstream filebuffer;
                    filebuffer << hmf_file.rdbuf();
                    rc = rhopsan.sendModelMessage(filebuffer.str());
                    rhopsan.requestMessages();

                    hmf_file.close();

                    if (rc)
                    {
                        rc = rhopsan.sendSimulateMessage(-1, -1, -1, -1, -1);
                        rhopsan.requestMessages();
                        if (rc)
                        {
                            vector<ResultVariableT> vars;
                            rc = rhopsan.requestSimulationResults(vars);
                            cout << PRINTCLIENT << "Results: " << rc << endl;
                        }
                        else
                        {
                            cout << PRINTCLIENT << "Server could not start the simulation" << endl;
                            simulationOK=false;
                        }
                    }
                    else
                    {
                        cout << PRINTCLIENT << "Server could not load model" << endl;
                        simulationOK=false;
                    }
                }

                // Execute shell commands
                bool shellExecOK=true;
                const std::vector<std::string> &rShellcommands =  shellOptions.getValue();
                for (const string &rCommand: rShellcommands)
                {
                    cout << PRINTCLIENT << "Remote executing shell command: " << rCommand <<  " ...";
                    std::string output;
                    bool rc = rhopsan.executeShellCommand(rCommand, output);
                    if (rc)
                    {
                        if(!nonBlockingShell.getValue())
                        {
                            bool printnewline=true;
                            auto startT = std::chrono::steady_clock::now();
                            WorkerStatusT status;
                            do
                            {
                                rhopsan.requestWorkerStatus(status);
                                if(status.shell_inprogress)
                                {
                                    if (printnewline)
                                    {
                                        cout << endl;
                                        printnewline = false;
                                    }
                                    else
                                    {
                                        cout << "\r" << "Still running after: " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-startT).count() << " seconds" << flush;
                                    }
                                    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(5000));
                                }
                            }while(status.shell_inprogress && !s_interrupted);
                            if (!s_interrupted)
                            {
                                if (status.shell_exitok)
                                {
                                    cout << " Success!" << endl;
                                    shellExecOK = true;
                                }
                                else
                                {
                                    cout << " Failed!   Did not exit OK" << endl;
                                    shellExecOK = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        cout << " Failed!   " << output << endl;
                        shellExecOK = false;
                    }

                    if (!shellExecOK)
                    {
                        break;
                    }
                }

                // Request results (files)
                bool fileRequestOK=true;
                const std::vector<std::string> &rRequests = requestOptions.getValue();
                for (const string &rRequest : rRequests)
                {
                    if (s_interrupted)
                    {
                        break;
                    }

                    cout << PRINTCLIENT << "Requesting: " << rRequest <<  " ... ";
                    double progress;
                    bool rc = rhopsan.blockingRequestFile(rRequest, rRequest, &progress);
                    fileRequestOK = fileRequestOK && rc;
                    cout << "Done!" << endl;
                }

                cout << PRINTCLIENT << "Sending goodby message!" << endl;
                rhopsan.disconnect();

                completedOK = (simulationOK && shellExecOK && fileRequestOK);
            }
            else
            {
                cout << PRINTCLIENT << "Could not get a server slot! Because: " << rhopsan.getLastErrorMessage() << endl;
                completedOK = false;
            }
        }
        catch (zmq::error_t e)
        {
            cout << PRINTCLIENT << "Error: " << e.what() << endl;
        }

        if (completedOK)
        {
            return 0;
        }
        else
        {
            return 1;
        }

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << PRINTCLIENT << "Error: " << e.error() << " for argument " << e.argId() << std::endl;
        std::cout << PRINTCLIENT << "Error: " << e.error() << " for argument " << e.argId() << std::endl;

        return 1;
    }
}

