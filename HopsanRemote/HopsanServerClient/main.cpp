//$Id$

#include <unistd.h>

#include "RemoteHopsanClient.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
using namespace std;

#define PRINTCLIENT "Client; "

int main(int argc, char* argv[])
{
    try {
        TCLAP::CmdLine cmd("RemoteHopsanClient");

        // Define a value argument and add it to the command line.
        TCLAP::MultiArg<std::string> shellOptions("", "shellexec", "Command to execute in shell", false, "string", cmd);
        TCLAP::MultiArg<std::string> requestOptions("", "request", "Request file (only from WD)", false, "string", cmd);
        TCLAP::MultiArg<std::string> assetsOptions("a", "asset", "Model assets (files)", false, "string (filepath)", cmd);
        TCLAP::ValueArg<std::string> userOption("u","user","The user identification string",false,"","user:password or user", cmd);
        TCLAP::ValueArg<std::string> hmfPathOption("m","hmf","The Hopsan model file to load",false,"","Path to file", cmd);
        TCLAP::ValueArg<std::string> serverAddrOption("s", "serverip", "Server IP address and port (default is localhost:45050)", false, "localhost:45050", "IP address", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        cout << PRINTCLIENT << "Connecting to: " << serverAddrOption.getValue() << endl;

        // Prepare our context and socket
#ifdef _WIN32
        zmq::context_t context(1, 63);
#else
        zmq::context_t context(1);
#endif
        RemoteHopsanClient rhopsan(context);
        rhopsan.connectToServer(serverAddrOption.getValue());
        cout << PRINTCLIENT << "Connected: " << rhopsan.serverConnected() << endl;

        try
        {
            int workerPort;
            bool rc = rhopsan.requestSlot(1, workerPort);
            if (rc)
            {
                cout << PRINTCLIENT << "Got server worker slot at port: " << workerPort << endl;
                rhopsan.connectToWorker(workerPort);

                // Send user identification
                if (userOption.isSet())
                {
                    string nameandpasswd = userOption.getValue();
                    string username = nameandpasswd;
                    string password;
                    size_t e = nameandpasswd.find_last_of(':');
                    if (e != string::npos)
                    {
                        username = nameandpasswd.substr(0, e);
                        password = nameandpasswd.substr(e+1);
                    }

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
                if (hmfPathOption.isSet())
                {
                    ifstream hmf_file(hmfPathOption.getValue());
                    if (!hmf_file.is_open())
                    {
                        cout << PRINTCLIENT << "Error: Could not open model file " << hmfPathOption.getValue() << endl;
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
                        }
                    }
                    else
                    {
                        cout << PRINTCLIENT << "Server could not load model" << endl;
                    }
                }

                // Execute shell commands
                const std::vector<std::string> &rShellcommands =  shellOptions.getValue();
                for (const string &rCommand: rShellcommands)
                {
                    cout << PRINTCLIENT << "Remote executing shell command: " << rCommand <<  " ...";
                    std::string output;
                    bool rc = rhopsan.executeShellCommand(rCommand, output);
                    if (rc)
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
                                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1000));
                            }
                        }while(status.shell_inprogress);
                        if (status.shell_exitok)
                        {
                            cout << " Success!" << endl;
                        }
                        else
                        {
                            cout << " Failed!   Did not exit OK" << endl;
                        }
                    }
                    else
                    {
                        cout << " Failed!   " << output << endl;
                    }
                }

                // Request results (files)
                const std::vector<std::string> &rRequests = requestOptions.getValue();
                for (const string &rRequest : rRequests)
                {
                    cout << PRINTCLIENT << "Requesting: " << rRequest <<  " ... ";
                    double progress;
                    rhopsan.blockingRequestFile(rRequest, rRequest, &progress);
                    cout << "Done!" << endl;
                }

                cout << PRINTCLIENT << "Sending goodby message!" << endl;
                rhopsan.disconnect();
            }
            else
            {
                cout << PRINTCLIENT << "Could not get a server slot! Because: " << rhopsan.getLastErrorMessage() << endl;
            }
        }
        catch (zmq::error_t e)
        {
            cout << PRINTCLIENT << "Error: " << e.what() << endl;
        }

        return 0;

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << PRINTCLIENT << "Error: " << e.error() << " for argument " << e.argId() << std::endl;
        std::cout << PRINTCLIENT << "Error: " << e.error() << " for argument " << e.argId() << std::endl;

        return 1;
    }
}

