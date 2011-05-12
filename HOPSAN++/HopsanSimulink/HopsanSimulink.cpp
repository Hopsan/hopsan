#define S_FUNCTION_NAME HopsanSimulink
#define S_FUNCTION_LEVEL 2
#define WIN32

#include "simstruc.h"
#include "..\HopsanCore\HopsanCore.h"
#include "HopsanWrapper.h"

extern double doTheMult(double, double);

static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) 
    {
        return;
    }

    //Define S-function input signals
    if (!ssSetNumInputPorts(S, 1)) return;
    ssSetInputPortWidth(S, 0, DYNAMICALLY_SIZED);
    ssSetInputPortDirectFeedThrough(S, 0, 1);
    //ssSetInputPortWidth(S, 1, DYNAMICALLY_SIZED);
    //ssSetInputPortDirectFeedThrough(S, 1, 1);

    //Define S-function output signals
    if (!ssSetNumOutputPorts(S,1)) return;
    ssSetOutputPortWidth(S, 0, DYNAMICALLY_SIZED);
    //ssSetOutputPortWidth(S, 1, DYNAMICALLY_SIZED);

    ssSetNumSampleTimes(S, 1);
    
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
    
    initSystem(1e-3);
    addComponent("Source", "SignalSource");
    addComponent("Subtract", "SignalSubtract");
    addComponent("Gain", "SignalGain");
    addComponent("Valve", "Hydraulic43Valve");
    addComponent("Cylinder", "HydraulicCylinderC");
    addComponent("Mass", "MechanicTranslationalMass");
    addComponent("Force", "MechanicForceTransformer");
    addComponent("Sensor", "MechanicPositionSensor");
    addComponent("Volume", "HydraulicVolume3");
    addComponent("Pump", "HydraulicFixedDisplacementPump");
    addComponent("PRV", "HydraulicPressureReliefValve");
    addComponent("Tank1", "HydraulicTankC");
    addComponent("Tank2", "HydraulicTankC");
    addComponent("Tank3", "HydraulicTankC");

      connect("Source", "out", "Subtract", "in1");
    connect("Sensor", "out", "Subtract", "in2");
    connect("Subtract", "out", "Gain", "in");
    connect("Gain", "out", "Valve", "in");
    connect("Valve", "PA", "Cylinder", "P1");
    connect("Valve", "PB", "Cylinder", "P2");
    connect("Cylinder", "P3", "Mass", "P1");
    connect("Mass", "P2", "Force", "P1");
    connect("Mass", "P2", "Sensor", "P1");
    connect("Volume", "P3", "Valve", "PP");
    connect("Valve", "PT", "Tank3", "P1");
    connect("Volume", "P2", "Pump", "P2");
    connect("Volume", "P1", "PRV", "P1");
    connect("Pump", "P1", "Tank1", "P1");
    connect("PRV", "P2", "Tank2", "P1");

    setParameter("Cylinder", "m_e", 1);
    setParameter("Cylinder", "c_leak", 0.00000001);
    setParameter("Pump", "n_p", 250);
    setParameter("Pump", "C_l,p", 0);
    setParameter("PRV", "p_ref", 20000000);
    setParameter("Gain", "k", 0.01);
    setParameter("Mass", "x_min", 0.0);
    setParameter("Mass", "x_max", 1.0);

    initComponents();
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
    setParameter("Source", "y", input);
    simulateOneTimestep(ssGetT(S));
    double output = getNodeData("Mass", "P2", 2);
      
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
