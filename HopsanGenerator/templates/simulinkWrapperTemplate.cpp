/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#define S_FUNCTION_NAME <<<name>>>
#define S_FUNCTION_LEVEL 2
#define MDL_START

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

#define NUMPARAMS 0

<<<15>>>
static void mdlInitializeSizes(SimStruct *S)
{
    // If NUMPARAMS != 0 then the mask setup will not work, so we can not use S-function parameters, for the parameters
    // instead we let the mask create local workspace variables corresponding to our system parameters
    ssSetNumSFcnParams(S, NUMPARAMS);
#if defined(MATLAB_MEX_FILE)
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
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

    //Load Hopsan model
    const char* hmfFilePath = "<<<4>>>";
    double startT, stopT; //Unused, but needed by Hopsan API
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
    pComponentSystem->setDesiredTimestep(<<<timestep>>>);

<<<5>>>}


static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, 0.001);
    ssSetOffsetTime(S, 0, 0.0);
}

static void mdlStart(SimStruct *S)
{
    //Update tunable parameters
    const mxArray* in;
    mwSize parsize;
    void* pBuffer=NULL;
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
    if (pBuffer != NULL) {
        free(pBuffer);
    }
}


static void mdlOutputs(SimStruct *S, int_T tid)
{
    //S-function input signals
    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);

    //S-function output signals
<<<7>>>
    //Read input variables from Simulink and write them to Hopsan
<<<8>>>
    //Simulate Hopsan until it reaches current Simulink time
    double time = ssGetT(S);
    pComponentSystem->simulate(time);

    //Read output variables from Hopsan and write them to Simulink
<<<12>>>
    //Read messages from Hopsan and print them
    while(gHopsanCore.checkMessage()) {
        HString msg, type, tag;
        gHopsanCore.getMessage(msg, type, tag);
        if (type != "debug") {
            ssPrintf("%s\n",msg.c_str());
        }
    }
}

static void mdlTerminate(SimStruct *S)
{
    //Execute finalize code in Hopsan at end of Simulation
    pComponentSystem->finalize();
}


 /* Simulink/Simulink Coder Interfaces */
 #ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
 #include "simulink.c" /* MEX-file interface mechanism */
 #else
 #include "cg_sfun.h" /* Code generation registration function */
 #endif

