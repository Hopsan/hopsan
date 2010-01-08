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
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSource", ComponentExternalSource::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalGain", ComponentExternalGain::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSink", ComponentExternalSink::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalStep", ComponentExternalStep::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSineWave", ComponentExternalSineWave::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalSquareWave", ComponentExternalSquareWave::Creator);
    cfact_ptr->RegisterCreatorFunction("ComponentExternalRamp", ComponentExternalRamp::Creator);

    //Mechanical components

}
