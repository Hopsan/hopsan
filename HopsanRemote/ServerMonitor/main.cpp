#include <iostream>
#include <iomanip>
#include <string>
#include "RemoteHopsanClient.h"

using namespace std;


//static int s_interrupted = 0;
//#ifdef _WIN32
//BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
//{
//    // what to do here?
//    s_interrupted = 1;
//    return TRUE;
//}
//#else
//static void s_signal_handler(int signal_value)
//{
//    s_interrupted = 1;
//}

//static void s_catch_signals(void)
//{
//    struct sigaction action;
//    action.sa_handler = s_signal_handler;
//    action.sa_flags = 0;
//    sigemptyset (&action.sa_mask);
//    sigaction (SIGINT, &action, NULL);
//    sigaction (SIGTERM, &action, NULL);
//}
//#endif

void printInfoFromAddressServer(vector<string> &ips, vector<string> &ports, vector<int> nslots, vector<double> speeds)
{
    cout << endl << endl;
    cout << std::setiosflags(std::ios::left) << std::setw(24) << "Adddress:" << std::setw(10) << "Slots:" << std::setw(10) << "Speed:" << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    for (size_t i=0; i<ips.size(); ++i)
    {
        string addr = ips[i]+":"+ports[i];
        cout << std::setw(24) << addr << std::setw(10) << nslots[i] << std::setw(10) << speeds[i] << endl;
    }
}


int main(int argc, char* argv[])
{
    string addressServerAddress, addressServerPort;

    if (argc < 3)
    {
        cout << /*PRINTWORKER << nowDateTime() <<*/ " Error: To few arguments!" << endl;
        return 1;
    }

    addressServerAddress = argv[1];
    addressServerPort = argv[2];

    cout << "Starting server monitor, connecting to: " << addressServerAddress << ":" << addressServerPort << endl;

    // Prepare our context
    try
    {
#ifdef _WIN32
        zmq::context_t context(1, 63);
#else
        zmq::context_t context(1);
#endif
        RemoteHopsanClient rhc(context);
        bool rc = rhc.connectToServer(addressServerAddress, addressServerPort);
        if (rc && rhc.serverConnected())
        {

//#ifdef _WIN32
//            SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
//#else
//            s_catch_signals();
//#endif
            bool keepRunning=true;
            while (keepRunning)
            {
                string input;
                cin >> input;

                // If update
                if (input == "u")
                {
                    vector<string> ips, ports;
                    vector<int> nslots;
                    vector<double> speeds;
                    bool rsm_rc = rhc.requestServerMachines(-1, 1e150, ips, ports, nslots, speeds);
                    if (rsm_rc)
                    {
                        printInfoFromAddressServer(ips,ports,nslots,speeds);
                    }
                    else
                    {
                        cout << "Error: could not get servers" << endl;
                    }
                }
                // Quit
                else if (input == "q")
                {
                    keepRunning=false;
                }

            }
        }
        else
        {
            cout << "Error: Failed to connect to address server!" << endl;
        }
    }
    catch(zmq::error_t e)
    {
        cout /*<< PRINTWORKER << nowDateTime()*/ << " Error: Preparing our context: " << e.what() << endl;
    }

    cout /*<< PRINTWORKER << nowDateTime()*/ << " Closed!" << endl;

    return 0;
}

