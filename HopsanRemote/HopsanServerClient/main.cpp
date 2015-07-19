//$Id$

#include <unistd.h>

#include "RemoteHopsanClient.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#define PRINTCLIENT "Client; "

int main(int argc, char* argv[])
{
    try {
        TCLAP::CmdLine cmd("RemoteHopsanClient", ' ');

        // Define a value argument and add it to the command line.
        TCLAP::ValueArg<std::string> serverAddrOption("s", "serverip", "Server IP address (default is localhost)", false, "localhost", "IP address", cmd);
        TCLAP::ValueArg<std::string> serverPortOption("p", "port", "Server port", true, "", "Port number", cmd);
        TCLAP::ValueArg<std::string> hmfPathOption("m","hmf","The Hopsan model file to load",false,"","Path to file", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        cout << PRINTCLIENT << "Connecting to: " << serverAddrOption.getValue() << ":" << serverPortOption.getValue()  << endl;

        // Prepare our context and socket
#ifdef _WIN32
        zmq::context_t context(1, 63);
#else
        zmq::context_t context(1);
#endif
        RemoteHopsanClient rhopsan(context);
        rhopsan.connectToServer(serverAddrOption.getValue()+":"+serverPortOption.getValue());
        int serverPort = atoi(serverPortOption.getValue().c_str());

        cout << PRINTCLIENT << "Connected: " << rhopsan.serverConnected() << endl;

        try
        {
            int ctrlPort;
            bool rc = rhopsan.requestSlot(1, ctrlPort);
            if (rc)
            {
                size_t workerPort = serverPort + ctrlPort;
                cout << PRINTCLIENT << "Got server worker slot at port: " << workerPort << endl;
                rhopsan.connectToWorker(ctrlPort);

                // Read model
                ifstream hmf_file(hmfPathOption.getValue());
                if (!hmf_file.is_open())
                {
                    cout << PRINTCLIENT << "Error: Could not open file " << hmfPathOption.getValue() << endl;
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
                        vector<string> names;
                        vector<double> data;
                        rc = rhopsan.requestSimulationResults(&names, &data);
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

