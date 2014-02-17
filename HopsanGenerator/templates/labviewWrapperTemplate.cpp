// Code from exported Hopsan model. This can be used in conjunction with HopsanCore by using HopsanWrapper. Subsystems probably don't work.


#include "hopsanrt-wrapper.h"
#include "SIT_API.h"
#include "model.h"
#include <stddef.h>
#include <math.h>
#include "codegen.c"

#define rtDBL	0

extern Parameters rtParameter[2];
extern long READSIDE;

#define readParam rtParameter[READSIDE]

typedef struct 
{
<<<inports>>>} Inports;

typedef struct
{
<<<outports>>>} Outports;

typedef struct
{
    double Time;
} Signals;

Inports rtInport;
Outports rtOutport;
Signals rtSignal;

long SetValueByDataType(void* ptr, int subindex, double value, int type)
{
    switch (type)
    {
        case rtDBL:
        ((double *)ptr)[subindex] = (double)value;
        return NI_OK;
    }
    return NI_ERROR;
}

double GetValueByDataType(void* ptr, int subindex, int type)
{
    switch (type)
    {
        case rtDBL:
        return (double)(((double *)ptr)[subindex]);
    }
    return 0x7FFFFFFFFFFFFFFF; /* NAN */
}

const long ParameterSize = 1;
const ParameterAttributes rtParamAttribs[] = 
{
    { "HopsanRT/sine/Amplitude", offsetof(Parameters, HopsanRT_sine_Amp), rtDBL, 1, 1}
};

const Parameters initParams = {0.0 /*time*/};

const long SignalSize = 1;
const SignalAttributes rtSignalAttribs[] = 
{
    { "HopsanRT/Time", 0, "Time", &rtSignal.Time, rtDBL, 1, 1}
};

const long InportSize = <<<inportsize>>>;
const ExtIOAttributes rtInportAttribs[] = 
{
<<<inportattribs>>>};

const long OutportSize = <<<outportsize>>>;
const ExtIOAttributes rtOutportAttribs[] = 
{
<<<outportattribs>>>};

const char * const ModelName = "HopsanRT";
const char * const build = "5.0.1 SIT Custom DLL";

const double baserate = .001;

long USER_Initialize() 
{
    createSystem(1e-3);

<<<components>>>
<<<connections>>>
<<<parameters>>>
    initSystem();
    rtSignal.Time = 0;

    return NI_OK;
}

void USER_TakeOneStep(double *inData, double *outData, double timestamp)
{
    rtSignal.Time += 0.001;
    if (inData)
    {
<<<indata>>>    }
    
<<<writenodedata>>>
    simulateOneTimestep(rtSignal.Time);

<<<readnodedata>>>    
    if (outData)
    {
<<<outdata>>>
    }
}

long USER_Finalize()
{
    return NI_OK;
}
