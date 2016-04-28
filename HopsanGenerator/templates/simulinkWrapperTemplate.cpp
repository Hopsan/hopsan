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

#define S_FUNCTION_NAME <<<name>>>
#define S_FUNCTION_LEVEL 2

#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include "simstruc.h"
#include "HopsanCore/include/HopsanCore.h"
using namespace hopsan;


HopsanEssentials gHopsanCore;
ComponentSystem* pComponentSystem;
bool isOkToSimulate = false;

#define NUMPARAMS 0//<<<numparams>>>

<<<15>>>
static void mdlInitializeSizes(SimStruct *S)
{
    // If NUMPARAMS != 0 then the mask setup will not work, so we can not use S-function parameters, for the parameters
    // instead we let the mask create local workspace variables corresponding to our system parameters
    ssSetNumSFcnParams(S, NUMPARAMS);
#if defined(MATLAB_MEX_FILE)
    if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)) {
        //mdlCheckParameters(S);
        //if (ssGetErrorStatus(S) != NULL) {
        //    return;
        //}
    } else {
        return; /* Parameter mismatch will be reported by Simulink. */
    }
#endif

    //Define S-function input signals
    if (!ssSetNumInputPorts(S,<<<0>>>)) return;				//Number of input signals
<<<1>>>
    //Define S-function output signals
    if (!ssSetNumOutputPorts(S,<<<2>>>)) return;				//Number of output signals
<<<3>>>
    ssSetOutputPortWidth(S, <<<14>>>, DYNAMICALLY_SIZED);		//Debug output signal
    ssSetNumSampleTimes(S, 1);
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);

    if(!gHopsanCore.hasComponent("HydraulicLaminarOrifice"))
    {
        ssSetErrorStatus(S, "Error: Component libraries failed to load.");
        return;
    }

//    std::vector<std::string> extLibs;
//    readExternalLibsFromTxtFile("externalLibs.txt",extLibs);
//    for (size_t i=0; i<extLibs.size(); ++i)
//    {
//        gHopsanCore.loadExternalComponentLib(extLibs[i].c_str());
//    }

    const char* hmfFilePath = "<<<4>>>";
    double startT, stopT;
    pComponentSystem = gHopsanCore.loadHMFModelFile(hmfFilePath, startT, stopT);
    if (pComponentSystem==0)
    {
        while(gHopsanCore.checkMessage())
        {
            HString msg, type, tag;
            gHopsanCore.getMessage(msg, type, tag);
            ssPrintf("%s\n",msg.c_str());
        }
        ssSetErrorStatus(S,"Error could not open model: <<<4>>>");
        return;
    }
    startT = ssGetTStart(S);
    stopT = ssGetTFinal(S);
    pComponentSystem->setDesiredTimestep(<<<timestep>>>);
<<<5>>>}


static void mdlInitializeSampleTimes(SimStruct *S)
{
    //ssSetSampleTime(S, 0, 0.001);
    //ssSetOffsetTime(S, 0, 0.0);

    //Update tunable parameters
    const mxArray* in;
    mwSize parsize;
    void* pBuffer=0;
    HString valstr;

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
        while(gHopsanCore.checkMessage())
        {
            HString msg, type, tag;
            gHopsanCore.getMessage(msg, type, tag);
            ssPrintf("%s\n",msg.c_str());
        }
        ssSetErrorStatus(S,"Error isSimulationOk() returned False! Most likely some components could not be loaded or some connections could not be established.");
        return;
    }

<<<16>>>

    // Free parameter buffer memory
    if (pBuffer)
    {
        free(pBuffer);
    }
}


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
    double time = ssGetT(S);
    pComponentSystem->simulate(time);

<<<12>>>

    //Output parameters
<<<13>>>
    while(gHopsanCore.checkMessage())
    {
        HString msg, type, tag;
        gHopsanCore.getMessage(msg, type, tag);
        if (type != "debug")
        {
            ssPrintf("%s\n",msg.c_str());
        }
    }
}

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

