#include "HopsanEssentials.h"
#include "CoreUtilities/FileAccess.h"
#include <iostream>
#include <string>

#include <tclap/CmdLine.h>
#include "../Utilities/TicToc.h"


using namespace std;
using namespace hopsan;

int main(int argc, char *argv[])
{

    cout << "Hello world!" << endl;

    try {
        TCLAP::CmdLine cmd("HopsanCLI", ' ', "put_version-number_here");

        // Define a value argument and add it to the command line.
        TCLAP::ValueArg<std::string> hmfPathOption("f","hmf","The Hopsan model file to simulate",false,"../Models/pressurerelifepaper.hmf","String containing file path", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        string hmfFilePath = hmfPathOption.getValue();

        FileAccess coreHmfLoader;

        double startTime=0, stopTime=2;
        ComponentSystem* pRootSystem = coreHmfLoader.loadModel(hmfFilePath, startTime, stopTime);

        TicToc initTimer("InitializeTime");
        pRootSystem->initialize(startTime, stopTime);
        initTimer.TocPrint();
        TicToc simuTimer("SimulationTime");
        pRootSystem->simulate(startTime, stopTime);
        simuTimer.TocPrint();

        cout << endl << "HopsanCLI Done!" << endl;

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cout << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}
