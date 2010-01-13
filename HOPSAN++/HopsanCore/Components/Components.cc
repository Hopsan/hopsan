#include "Components.h"


DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr)
{
    //Hydraulic components
    cfact_ptr->RegisterCreatorFunction("HydraulicLaminarOrifice", HydraulicLaminarOrifice::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicVolume", HydraulicVolume::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureSource", HydraulicPressureSource::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicFlowSourceQ", HydraulicFlowSourceQ::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureSourceQ", HydraulicPressureSourceQ::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicFixedDisplacementPump", HydraulicFixedDisplacementPump::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicCheckValve", HydraulicCheckValve::Creator);
    cfact_ptr->RegisterCreatorFunction("Hydraulic43Valve", Hydraulic43Valve::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicVariableDisplacementPump", HydraulicVariableDisplacementPump::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicAckumulator", HydraulicAckumulator::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureControlledValve", HydraulicPressureControlledValve::Creator);

    //Signal components
    cfact_ptr->RegisterCreatorFunction("SignalSource", SignalSource::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalGain", SignalGain::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSink", SignalSink::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalStep", SignalStep::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSineWave", SignalSineWave::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSquareWave", SignalSquareWave::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalRamp", SignalRamp::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalAdd", SignalAdd::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSubtract", SignalSubtract::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalMultiply", SignalMultiply::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalDivide", SignalDivide::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSaturation", SignalSaturation::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalDeadZone", SignalDeadZone::Creator);


    //Mechanical components
    cfact_ptr->RegisterCreatorFunction("MechanicForceTransformer", MechanicForceTransformer::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicVelocityTransformer", MechanicVelocityTransformer::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicTranslationalMass", MechanicTranslationalMass::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicTranslationalSpring", MechanicTranslationalSpring::Creator);

}
