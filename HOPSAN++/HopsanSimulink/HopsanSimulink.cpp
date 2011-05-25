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

#include "simstruc.h"
#include <sstream>
#include "../HopsanCore/HopsanCore.h"
#include "../HopsanCore/Component.h"
#include "../HopsanCore/ComponentSystem.h"
#include "../HopsanCore/ComponentEssentials.h"
#include "../HopsanCore/ComponentUtilities.h"
#include "../HopsanCore/HopsanEssentials.h"
#include "../HopsanCore/Node.h"
#include "../HopsanCore/Port.h"
#include "../HopsanCore/Nodes/Nodes.h"
#include "../HopsanCore/ComponentUtilities/AuxiliarySimulationFunctions.h"
#include "../HopsanCore/ComponentUtilities/CSVParser.h"
#include "../HopsanCore/ComponentUtilities/Delay.h"
#include "../HopsanCore/ComponentUtilities/DoubleIntegratorWithDamping.h"
#include "../HopsanCore/ComponentUtilities/FirstOrderFilter.h"
#include "../HopsanCore/ComponentUtilities/Integrator.h"
#include "../HopsanCore/ComponentUtilities/IntegratorLimited.h"
#include "../HopsanCore/ComponentUtilities/ludcmp.h"
#include "../HopsanCore/ComponentUtilities/matrix.h"
#include "../HopsanCore/ComponentUtilities/ReadDataCurve.h"
#include "../HopsanCore/ComponentUtilities/SecondOrderFilter.h"
#include "../HopsanCore/ComponentUtilities/SecondOrderTransferFunction.h"
#include "../HopsanCore/ComponentUtilities/TurbulentFlowFunction.h"
#include "../HopsanCore/ComponentUtilities/ValveHysteresis.h"
#include "../HopsanCore/CoreUtilities/HmfLoader.h"
#include "../HopsanCore/CoreUtilities/ClassFactory.hpp"
#include "../HopsanCore/CoreUtilities/FindUniqueName.h"
#include "../HopsanCore/CoreUtilities/HopsanCoreMessageHandler.h"
#include "../HopsanCore/CoreUtilities/LoadExternal.h"
#include "../HopsanCore/Components/Components.h"

using namespace hopsan;

ComponentSystem* pComponentSystem; 

static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) 
    {
        return;
    }

    //Define S-function input signals
    if (!ssSetNumInputPorts(S, 1)) return;				//Number of input signals
    ssSetInputPortWidth(S, 0, DYNAMICALLY_SIZED);		//Input signal 0
    ssSetInputPortDirectFeedThrough(S, 0, 1);			
	//ssSetInputPortWidth(S, 1, DYNAMICALLY_SIZED);		//Input signal 1
    //ssSetInputPortDirectFeedThrough(S, 1, 1);			

    //Define S-function output signals
    if (!ssSetNumOutputPorts(S,1)) return;				//Number of output signals
    ssSetOutputPortWidth(S, 0, DYNAMICALLY_SIZED);		//Output signal 0
	//ssSetOutputPortWidth(S, 1, DYNAMICALLY_SIZED);		//Output signal 1

    ssSetNumSampleTimes(S, 1);
    
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
    
    std::string hmfFilePath = "Model.hmf";
    hopsan::HmfLoader coreHmfLoader;
    double startT = ssGetTStart(S);
    double stopT = ssGetTFinal(S);
    pComponentSystem = coreHmfLoader.loadModel(hmfFilePath, startT, stopT);
    pComponentSystem->setDesiredTimestep(0.001);
    std::stringstream ss;
    ss << "startT: " << startT << ", stopT: " << stopT << "\n";
    ssPrintf(ss.str().c_str());
    pComponentSystem->initializeComponentsOnly();

}

static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0.0);
}

static void mdlOutputs(SimStruct *S, int_T tid)
{
    //S-function input signals
    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);
      
    //S-function output signals
    real_T *y1 = ssGetOutputPortRealSignal(S,0);
    int_T width1 = ssGetOutputPortWidth(S,0);
    
    //Input parameters
    double input = (*uPtrs1[0]);
    
    //Equations
    double output;
    if(pComponentSystem == 0)
    {
      output = -1;		//Error code -1: Component system failed to load
    }
    else if(!pComponentSystem->isSimulationOk())
    {
      output = -2;		//Error code -2: Simulation not possible due to errors in model
    }
    else
    {
        pComponentSystem->getComponent("Source")->setParameterValue("y", input);
        double timestep = pComponentSystem->getDesiredTimeStep();
        double time = ssGetT(S);
        pComponentSystem->simulate(time, time+timestep);
        
        std::stringstream ss6;
        ss6 << "DEBUG5\n";
        ssPrintf(ss6.str().c_str());
        
        output = pComponentSystem->getComponent("Mass")->getPort("P2")->readNode(NodeMechanic::POSITION);     //NodeMechanic::POSITION = 2
        //output = pComponentSystem->getComponent("Mass")->getPort("P2")->getTimeVectorPtr()->size();
        //output = *pComponentSystem->getComponent("Mass")->getTimePtr();
        //output = 6;
        
        std::stringstream ss7;
        ss7 << "DEBUG6\n";
        ssPrintf(ss7.str().c_str());
    }


      
    //Output parameters
    *y1 = output;
    
    
    
    //These methods are used in case inputs/outputs are arrays
      //int_T i;
      //for (i=0; i<width1; i++) {
      //    *y1++ = 2.0 *(*uPtrs1[i]);
      //}
      //for (i=0; i<width2; i++) {
      //    *y2++ = 3.0 *(*uPtrs2[i]);
      //}
}

static void mdlTerminate(SimStruct *S){}




/* Simulink/Simulink Coder Interfaces */
#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#include "simulink.c" /* MEX-file interface mechanism */
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
