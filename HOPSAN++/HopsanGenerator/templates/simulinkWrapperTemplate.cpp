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

#define S_FUNCTION_NAME <<<name>>>
#define S_FUNCTION_LEVEL 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include "simstruc.h"
#include "HopsanCore/include/HopsanCore.h"
using namespace hopsan;


//! @todo need to be able to error report if file not fond, or maybe not, if no external libs used you dont want error message
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames)
{
    rExtLibFileNames.clear();
    std::string line;
    std::ifstream file;
    file.open(filePath.c_str());
    if ( file.is_open() )
    {
        while ( file.good() )
        {
            getline(file, line);
            if ((*line.begin() != '#') && !line.empty())
            {
                rExtLibFileNames.push_back(line);
            }
       }
        file.close();
    }
    else
    {
        std::cout << "error, could not open file: " << filePath << std::endl;
    }
}


HopsanEssentials gHopsanCore;
ComponentSystem* pComponentSystem;
bool isOkToSimulate = false;

<<<15>>>
static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    {
        return;
    }

    //Define S-function input signals
    if (!ssSetNumInputPorts(S,<<<0>>>)) return;				//Number of input signals
<<<1>>>
    //Define S-function output signals
    if (!ssSetNumOutputPorts(S,<<<2>>>)) return;				//Number of output signals
<<<3>>>
    ssSetOutputPortWidth(S, <<<14>>>, DYNAMICALLY_SIZED);		//Debug output signal
    ssSetNumSampleTimes(S, 1);
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
  
    std::vector<std::string> extLibs;
    readExternalLibsFromTxtFile("externalLibs.txt",extLibs);
    for (size_t i=0; i<extLibs.size(); ++i)
    {
        gHopsanCore.loadExternalComponentLib(extLibs[i].c_str());
    }

    const char* hmfFilePath = "<<<4>>>";
    double startT, stopT;
    pComponentSystem = gHopsanCore.loadHMFModel(hmfFilePath, startT, stopT);
    if (pComponentSystem==0)
    {
        ssSetErrorStatus(S,"Error could not open model: <<<4>>>");
        return;
    }
    startT = ssGetTStart(S);
    stopT = ssGetTFinal(S);
    pComponentSystem->setDesiredTimestep(0.001);
<<<5>>>}


static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, 0.001);
    ssSetOffsetTime(S, 0, 0.0);

    //Update tunable parameters
    const mxArray* in;
    const char* c_str;
    HString str;
<<<6>>>


    isOkToSimulate = pComponentSystem->checkModelBeforeSimulation();
    if (isOkToSimulate)
    {
        pComponentSystem->setNumLogSamples(0);
        pComponentSystem->disableLog();
        pComponentSystem->initialize(0,1);
    }
    else
    {
        ssSetErrorStatus(S,"Error isSimulationOk() returned False! Most likely some components could not be loaded or some connections could not be established.");
        return;
    }
    
<<<16>>>}


static void mdlOutputs(SimStruct *S, int_T tid)
{
    //S-function input signals
    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);

    //S-function output signals
    <<<7>>>
    int_T width1 = ssGetOutputPortWidth(S,0);

    //Input parameters
<<<8>>>
    //Equations
<<<9>>>
    output<<<10>>> = 0;		//Error code 0: Nothing is wrong

<<<11>>>
    double timestep = pComponentSystem->getDesiredTimeStep();
    double time = ssGetT(S);
    pComponentSystem->simulate(time+timestep);

<<<12>>>
    
    //Output parameters
<<<13>>>}
     
static void mdlTerminate(SimStruct *S)
{
    pComponentSystem->finalize();
}
     
     
 /* Simulink/Simulink Coder Interfaces */
 #ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
 #include "simulink.c" /* MEX-file interface mechanism */
 #else
 #include "cg_sfun.h" /* Code generation registration function */
 #endif

     
