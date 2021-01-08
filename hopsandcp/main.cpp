//#include "hopsandcp.h"
#include "dcpslave.h"

#include <tclap/CmdLine.h>

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    TCLAP::CmdLine cmd("hopsandcp", ' ', "0.1");

    // Define a value argument and add it to the command line.
    TCLAP::SwitchArg generateDescription("d","description","Generate DCP desciption file",cmd);
    TCLAP::ValueArg<std::string> argModelFile("m","model","Hopsan model file",false, "", "", cmd);
    TCLAP::ValueArg<std::string> argTargetDescriptionFile("t","target","Target file for DCP description",false, "", "", cmd);
    TCLAP::ValueArg<std::string> argHost("a","address","Host address",false,"127.0.0.1","",cmd);
    TCLAP::ValueArg<int> argPort("p","port","Port",false,8080,"",cmd);
    // Parse the argv array.
    cmd.parse( argc, argv );

    if(generateDescription.isSet()) {
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
        string modelFile = argModelFile.getValue();
        std::string host = argHost.getValue();
        int port = argPort.getValue();
        string targetFile;
        if(argTargetDescriptionFile.isSet()) {
            targetFile = argTargetDescriptionFile.getValue();
        }
        else {
            targetFile = argModelFile.getValue();
            targetFile = targetFile.substr(0,targetFile.find_last_of('.'))+".dcpx";
        }

        DcpSlave *pSlave = new DcpSlave(modelFile,host,port);
        pSlave->generateDescriptionFile(targetFile);
    }

    std::cout << "hopsandcp completed successfully!\n";

    return 0;
}
