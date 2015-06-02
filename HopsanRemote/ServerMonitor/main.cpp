#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <sstream>
#include <chrono>
#include "RemoteHopsanClient.h"

using namespace std;
using namespace std::chrono;
typedef duration<double> fseconds;

bool gKeepRunning = true;
double gRefreshTime = 10;

void inputThreadFunc()
{
    while (gKeepRunning)
    {
        string input;
        cin >> input;

        // If update
        if (input.front() == 'u')
        {
            stringstream ss;
            ss << input.substr(1);
            ss >> gRefreshTime;
        }
        // Quit
        else if (input.front() == 'q')
        {
            gKeepRunning=false;
        }
    }
}

std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return std::string(&buff[0]);
}

void printInfoFromAddressServer(vector<string> &ips, vector<string> &ports, vector<string> &descs, vector<int> &nslots, vector<int> &nopenslots, vector<double> &speeds)
{
    cout << endl << endl;
    cout << nowDateTime() << endl;
    cout << std::setiosflags(std::ios::left) << std::setw(24) << "Adddress:" << std::setw(10) << "Slots:" << std::setw(10) << "Speed:" << std::setw(10) << "Description:" << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    for (size_t i=0; i<ips.size(); ++i)
    {
        string addr = ips[i]+":"+ports[i];
        stringstream ss;
        ss << nopenslots[i] << "/" << nslots[i];

        cout << std::setw(24) << addr << std::setw(10) << ss.str() << std::setw(10) << speeds[i] << std::setw(10) << descs[i] << endl;
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

    cout << endl;
    cout << "Starting server monitor, connecting to: " << addressServerAddress << ":" << addressServerPort << endl;
    cout << "Enter  q  to quit or  uN  (where N is the refresh time in seconds)" << endl;

    thread inputThread(inputThreadFunc);

    // Prepare our context
    try
    {
#ifdef _WIN32
        zmq::context_t context(1, 63);
#else
        zmq::context_t context(1);
#endif
        RemoteHopsanClient rhc(context);
        RemoteHopsanClient rhcDirect(context);
        bool rc = rhc.connectToServer(addressServerAddress, addressServerPort);
        if (rc && rhc.serverConnected())
        {
            steady_clock::time_point lastRefreshTime;
            while (gKeepRunning)
            {
                fseconds dur = duration_cast<fseconds>(steady_clock::now() - lastRefreshTime);
                if (dur.count() > gRefreshTime)
                {

                    vector<string> ips, ports, desc;
                    vector<int> nslots, nopenslots;
                    vector<double> speeds;
                    bool rsm_rc = rhc.requestServerMachines(-1, 1e150, ips, ports, desc, nslots, speeds);

                    nopenslots.resize(nslots.size(), -1);

                    lastRefreshTime = chrono::steady_clock::now();
                    if (rsm_rc)
                    {
                        for (size_t i=0; i<ips.size(); ++i)
                        {
                            if (!rhcDirect.connectToServer(ips[i], ports[i]))
                            {
                                cout << rhcDirect.getLastErrorMessage() << endl;
                            }
                            if (rhcDirect.serverConnected())
                            {
                                ServerStatusT status;
                                bool gotStatus = rhcDirect.requestServerStatus(status);
                                if (gotStatus)
                                {
                                    nopenslots[i] = status.numFreeSlots;
                                }
                                else
                                {
                                    cout << "Error: Could not get status from server: " << ips[i] << ":" << ports[i] << endl;
                                }
                            }
                            else
                            {
                                cout << "Error: Could not connect to server: " << ips[i] << ":" << ports[i] << endl;
                            }
                            rhcDirect.disconnectServer();
                        }

                        printInfoFromAddressServer(ips,ports,desc,nslots,nopenslots,speeds);
                    }
                    else
                    {
                        cout << "Error: could not get servers" << endl;
                    }
                }
                else
                {
                    this_thread::sleep_for(fseconds(min(max(gRefreshTime-dur.count(),0.), 1.0)));
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

    gKeepRunning = false;
    inputThread.join();
    cout /*<< PRINTWORKER << nowDateTime()*/ << " Closed!" << endl;

    return 0;
}

