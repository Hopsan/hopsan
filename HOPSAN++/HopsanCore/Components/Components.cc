#include "Components.h"
#include "../Component.h"


DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr)
{
    //Hydraulic components
    cfact_ptr->RegisterCreatorFunction();
    cfact_ptr->RegisterCreatorFunction("ComponentExternalOrifice", ComponentExternalOrifice::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalVolume", ComponentExternalVolume::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalPressureSource", ComponentExternalPressureSource::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalFlowSourceQ", ComponentExternalFlowSourceQ::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalPressureSourceQ", ComponentExternalPressureSourceQ::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalFixedDisplacementPump", ComponentExternalFixedDisplacementPump::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalCheckValve", ComponentExternalCheckValve::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternal43Valve", ComponentExternal43Valve::Creator);

    //Signal components
    cfact_ptr->RegisterCreatorFunction("SignalSource", SignalSource::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalGain", SignalGain::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSink", SignalSink::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalStep", SignalStep::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSineWave", SignalSineWave::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSquareWave", SignalSquareWave::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalRamp", SignalRamp::Creator);

    //Mechanical components

}
