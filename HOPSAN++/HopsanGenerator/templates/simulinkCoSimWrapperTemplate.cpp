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

#define S_FUNCTION_NAME HopsanSimulink
#define S_FUNCTION_LEVEL 2

#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include "simstruc.h"
#include "include/HopsanCore.h"
#include "boost/boost/interprocess/shared_memory_object.hpp"
#include "boost/boost/interprocess/mapped_region.hpp"

using namespace hopsan;


bool *sim_socket;
bool *stop_socket;
<<<0>>>

static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    {
        return;
    }

    //Define S-function input signals
    if (!ssSetNumInputPorts(S,<<<1>>>)) return;				//Number of input signals\n
<<<2>>>

    //Define S-function output signals\n
    if (!ssSetNumOutputPorts(S,<<<3>>>)) return;				//Number of output signals\n
<<<4>>>
    ssSetOutputPortWidth(S, <<<5>>>, DYNAMICALLY_SIZED);		//Debug output signal\n
    ssSetNumSampleTimes(S, 1);
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
  
<<<6>>>
}


static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, 0.001);
    ssSetOffsetTime(S, 0, 0.0);

    //Initialize shared memory sockets

    //Input
<<<7>>>    
//Output
<<<8>>>    
//Stop
<<<9>>>    
//Simulate
<<<10>>>
    (*stop_socket) = false;
    (*sim_socket) = false;
}


static void mdlOutputs(SimStruct *S, int_T tid)
{
    //S-function input signals
    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);

    //S-function output signals
<<<11>>>
    int_T width1 = ssGetOutputPortWidth(S,0);

    //Input parameters
<<<12>>>

    //Equations
<<<13>>>
    output<<<14>>> = 0;		//Error code 0: Nothing is wrong

    //Write input values
<<<15>>>
    //Tell Hopsan to simulate
    (*sim_socket) = true;

    //Make sure step is completed by Hopsan
    while((*sim_socket)) {}  

    //Read output values
<<<16>>>
    
    //Output parameters
<<<17>>>}
     
 static void mdlTerminate(SimStruct *S)
{
    //Tell Hopsan to stop simulation
    (*stop_socket) = true;

    //Remove sockets
<<<18>>>
    boost::interprocess::shared_memory_object::remove("hopsan_sim"); 
    boost::interprocess::shared_memory_object::remove("hopsan_stop"); 
}
     
     
     /* Simulink/Simulink Coder Interfaces */
     #ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
     #include "simulink.c" /* MEX-file interface mechanism */
     #else
     #include "cg_sfun.h" /* Code generation registration function */
     #endif

     
