/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

#include "HopsanEssentials.h"
#include <iostream>
#include <string>
#include <vector>

#include <tclap/CmdLine.h>
#include "TicToc.hpp"

// If we dont have the revision number then define blank
#ifndef HOPSANCLISVNREVISION
#define HOPSANCLISVNREVISION "UNKNOWN"
#endif

#define HOPSANCLIVERSION "0.5.x_r" HOPSANCLISVNREVISION

#ifdef WIN32
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/components/libdefaulComponentLibrary.dll"
#else
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/components/libdefaulComponentLibrary.so"
#endif

using namespace std;
using namespace hopsan;

void printWaitingMessages()
{
    std::string msg,type,tag;
    cout << "Check messages: " << HopsanEssentials::getInstance()->checkMessage() << endl;
    while (HopsanEssentials::getInstance()->checkMessage() > 0)
    {
        HopsanEssentials::getInstance()->getMessage(msg,type,tag);
        cout << msg << endl;
    }
}

void printComponentHierarchy(ComponentSystem *pSystem, std::string prefix="")
{
    if (pSystem)
    {
        cout << prefix << pSystem->getName() << endl;
        prefix.append("  ");
        vector<string> names = pSystem->getSubComponentNames();
        for (size_t i=0; i<names.size(); ++i)
        {
            if ( pSystem->getSubComponent(names[i])->isComponentSystem() )
            {
                printComponentHierarchy(pSystem->getSubComponentSystem(names[i]), prefix);
            }
            else
            {
                cout << prefix << names[i] << endl;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    try {
        TCLAP::CmdLine cmd("HopsanCLI", ' ', HOPSANCLIVERSION);

        // Define a value argument and add it to the command line.
        TCLAP::ValueArg<std::string> hmfPathOption("f","hmf","The Hopsan model file to simulate",false,"","String containing file path", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        string hmfFilePath = hmfPathOption.getValue();

        // Load default hopasn component lib
        HopsanEssentials::getInstance()->loadExternalComponentLib(DEFAULTCOMPONENTLIB);
        printWaitingMessages();

        double startTime=0, stopTime=2;
        ComponentSystem* pRootSystem = HopsanEssentials::getInstance()->loadHMFModel(hmfFilePath, startTime, stopTime);
        printWaitingMessages();

        cout << endl << "Component Hieararcy:" << endl << endl;
        printComponentHierarchy(pRootSystem);
        cout << endl;

        if (pRootSystem!=0)
        {
            TicToc initTimer("InitializeTime");
            bool initSuccess = pRootSystem->initialize(startTime, stopTime);
            initTimer.TocPrint();
            if (initSuccess)
            {
                TicToc simuTimer("SimulationTime");
                pRootSystem->simulate(startTime, stopTime);
                simuTimer.TocPrint();
            }
            else
            {
                cout << "Initialize failed, Simulation aborted!" << endl;
            }
        }

        printWaitingMessages();
        cout << endl << "HopsanCLI Done!" << endl;

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cout << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}
