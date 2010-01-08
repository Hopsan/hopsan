#include "Components.h"
#include "../Component.h"


DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr)
{
    //Hydraulic components
    cfact_ptr->RegisterCreatorFunction();
    cfact_ptr->RegisterCreatorFunction("HydraulicLaminarOrifice", ComponentExternalOrifice::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicVolume", ComponentExternalVolume::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureSource", ComponentExternalPressureSource::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicFlowSourceQ", ComponentExternalFlowSourceQ::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureSourceQ", ComponentExternalPressureSourceQ::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicFixedDisplacementPump", ComponentExternalFixedDisplacementPump::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicCheckValve", ComponentExternalCheckValve::Creator);
    cfact_ptr->RegisterCreatorFunction("Hydraulic43Valve", ComponentExternal43Valve::Creator);

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
