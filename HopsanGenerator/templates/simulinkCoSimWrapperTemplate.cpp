/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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


double *sim_socket;
bool *stop_socket;
<<<0>>>

boost::interprocess::shared_memory_object shdmem_sim;
boost::interprocess::shared_memory_object shdmem_stop;
>>>19>>>boost::interprocess::shared_memory_object shdmem_<<<name>>>;
<<<19<<<

boost::interprocess::mapped_region region_sim;
boost::interprocess::mapped_region region_stop;
>>>20>>>boost::interprocess::mapped_region region_<<<name>>>;
<<<20<<<

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
    (*sim_socket) = 0.0;
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
    (*sim_socket) = 10.0;

    //Make sure step is completed by Hopsan
    while((*sim_socket) > 5.0) { mexEvalString("drawnow;"); }  

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

     
