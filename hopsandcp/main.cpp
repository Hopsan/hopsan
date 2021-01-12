#include "dcpslave.h"
#include "dcpmaster.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <deque>

using namespace std;

int main(int argc, char* argv[])
{
    TCLAP::CmdLine cmd("hopsandcp", ' ', "0.1");

    // Define a value argument and add it to the command line.
    TCLAP::SwitchArg argMakeDescription("d","description","Generate DCP desciption file",cmd);
    TCLAP::SwitchArg argSlave("s","slave","Execute simulation in slave mode",cmd);
    TCLAP::SwitchArg argMaster("m","master","Execute simulation in master mode",cmd);
    TCLAP::ValueArg<std::string> argModelFile("i","input","Input Hopsan model file",false, "", "", cmd);
    TCLAP::ValueArg<std::string> argTargetDescriptionFile("t","target","Target file for DCP description",false, "", "", cmd);
    TCLAP::ValueArg<std::string> argHost("a","address","Host address",false,"127.0.0.1","",cmd);
    TCLAP::ValueArg<int> argPort("p","port","Port",false,8080,"",cmd);
    TCLAP::MultiArg<std::string> argRemoteSlave("r","remote","Connect to remote slave",false,"",cmd);
    TCLAP::MultiArg<std::string> argConnect("c","connect","Add connection (fromSlave,fromVr,toSlave1,toVr1,toSlave2,toVr2...)",false,"",cmd);
    // Parse the argv array.
    cmd.parse( argc, argv );

    if((argMakeDescription.isSet() && (argSlave.isSet() || argMaster.isSet())) ||
        (argSlave.isSet() && (argMakeDescription.isSet() || argMaster.isSet())) ||
         (argMaster.isSet() && (argSlave.isSet() || argMakeDescription.isSet()))) {
        cout << "Cannot only run in one mode at a time (make description, slave or master).\n";
        return -1;
    }

    //Generate DCP description file
    if(argMakeDescription.isSet()) {
        if(!argModelFile.isSet()) {
            cout << "Generating a DCP description requires a model file.\n";
            return -1;
        }
        if(!argHost.isSet()) {
            cout << "Generating a DCP description requires a host address.\n";
            return -1;
        }
        if(!argPort.isSet()) {
            cout << "Generating a DCP description requires a port.\n";
            return -1;
        }
        string targetFile;
        if(argTargetDescriptionFile.isSet()) {
            targetFile = argTargetDescriptionFile.getValue();
        }
        else {
            targetFile = argModelFile.getValue();
            targetFile = targetFile.substr(0,targetFile.find_last_of('.'))+".dcpx";
        }

        DcpSlave *pSlave = new DcpSlave(argModelFile.getValue(),argHost.getValue(),argPort.getValue());
        pSlave->generateDescriptionFile(targetFile);
    }

    //Run a simulation as slave
    if(argSlave.isSet()) {
        if(!argModelFile.isSet()) {
            cout << "Running in slave mode requires a model file.\n";
            return -1;
        }
        if(!argHost.isSet()) {
            cout << "Running in slave mode requires a host address.\n";
            return -1;
        }
        if(!argPort.isSet()) {
            cout << "Running in slave mode requires requires a port.\n";
            return -1;
        }
        DcpSlave *pSlave = new DcpSlave(argModelFile.getValue(),argHost.getValue(),argPort.getValue());
        pSlave->start();
    }

    //Run a simulation as master
    if(argMaster.isSet()) {
        if(!argHost.isSet()) {
            cout << "Running in master mode requires a host address.\n";
            return -1;
        }
        if(!argPort.isSet()) {
            cout << "Running in master mode requires requires a port.\n";
            return -1;
        }

        DcpMaster *pMaster = new DcpMaster(argHost.getValue(), argPort.getValue());
        for(auto slave : argRemoteSlave.getValue()) {
            pMaster->addSlave(slave);
        }
        for(auto connection : argConnect.getValue()) {
            std::deque<size_t> ids;
            std::stringstream ss(connection);
            for (size_t i; ss >> i;) {
                ids.push_back(i);
                if (ss.peek() == ',') {
                    ss.ignore();
                }
            }
            if(ids.size() < 4) {
                cout << "A connection must have at least one sender and one receiver.\n";
                return -1;
            }

            size_t fromSlave = ids[0];
            size_t fromVr = ids[1];
            ids.pop_front();
            ids.pop_front();
            std::vector<size_t> toSlaves, toVrs;
            for(size_t i=0; i<ids.size(); ++i) {
                if(i%2 == 0) {
                    toSlaves.push_back(ids[i]);
                }
                else {
                    toVrs.push_back(ids[i]);
                }
            }

            pMaster->addConnection(fromSlave, fromVr, toSlaves, toVrs);
        }

        pMaster->start();
    }

    std::cout << "hopsandcp completed successfully!\n";

    return 0;
}
