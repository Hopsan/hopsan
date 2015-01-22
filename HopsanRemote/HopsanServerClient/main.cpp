//$Id$

#include <unistd.h>

#include "RemoteHopsanClient.h"

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

int main()
{
    cout << "Client Process! " << endl;

    // Read model
    ifstream hmf_file("./Position Servo.hmf");
    std::stringstream filebuffer;
    filebuffer << hmf_file.rdbuf();


    // Prepare our context and socket
    zmq::context_t context (1);
    RemoteHopsanClient rhopsan(context);
    rhopsan.connectToServer(makeAddress("localhost",23300));

    cout << "Connected: " << rhopsan.serverConnected() << endl;

    try
    {
        size_t ctrlPort;
        bool rc = rhopsan.requestSlot(ctrlPort);
        if (rc)
        {
            cout << "Server Worker slot at port: " << ctrlPort << endl;
            rhopsan.connectToWorker(makeAddress("localhost", ctrlPort));

            rc = rhopsan.sendModelMessage(filebuffer.str());
            rhopsan.requestMessages();
            if (rc)
            {
                rc = rhopsan.sendSimulateMessage(-1, -1, -1, -1, -1);
                rhopsan.requestMessages();
                if (rc)
                {
                    vector<string> names;
                    vector<double> data;
                    rc = rhopsan.requestSimulationResults(names, data);
                    cout << "Results: " << rc << endl;
                }
                else
                {
                    cout << "Could not start the simulation" << endl;
                }
            }
            else
            {
                cout << "Server could not load model" << endl;
            }

            cout << "Sending goodby message!" << endl;
            rhopsan.disconnect();
        }
        else
        {
            cout << "Server denied slot request!" << endl;
        }
    }
    catch (zmq::error_t e)
    {
        cout << "Client error: " << e.what() << endl;
    }



    hmf_file.close();

    return 0;
}

