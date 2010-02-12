//!
//! @file   Components.cc
//! @author <FluMeS>
//! @date   2010-01-08
//! @brief Contains the register_components function that registers all built in components
//!
//$Id$

#include "Components.h"

//!
//! @brief Registers the creator function of all built in components
//! @param [in,out] cfact_ptr A pointer the the component factory in wich to register the components
//!
DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr)
{
    //Hydraulic components
    cfact_ptr->RegisterCreatorFunction("HydraulicLaminarOrifice", HydraulicLaminarOrifice::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicTurbulentOrifice", HydraulicTurbulentOrifice::Creator);
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
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureSensor", HydraulicPressureSensor::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicFlowSensor", HydraulicFlowSensor::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPowerSensor", HydraulicPowerSensor::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicCylinderC", HydraulicCylinderC::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicCylinderQ", HydraulicCylinderQ::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicPressureReliefValve", HydraulicPressureReliefValve::Creator);
    cfact_ptr->RegisterCreatorFunction("HydraulicSubSysExample", HydraulicSubSysExample::Creator);


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
    cfact_ptr->RegisterCreatorFunction("SignalLP1Filter", SignalLP1Filter::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalLP2Filter", SignalLP1Filter::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalPulse", SignalPulse::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSoftStep", SignalSoftStep::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalIntegrator", SignalIntegrator::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalIntegrator2", SignalIntegrator2::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalIntegratorLimited", SignalIntegratorLimited::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalIntegratorLimited2", SignalIntegratorLimited2::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalTimeDelay", SignalTimeDelay::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalFirstOrderFilter", SignalFirstOrderFilter::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalSecondOrderFilter", SignalSecondOrderFilter::Creator);
    cfact_ptr->RegisterCreatorFunction("SignalHysteresis", SignalHysteresis::Creator);

    //Mechanical components
    cfact_ptr->RegisterCreatorFunction("MechanicForceTransformer", MechanicForceTransformer::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicVelocityTransformer", MechanicVelocityTransformer::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicTranslationalMass", MechanicTranslationalMass::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicTranslationalSpring", MechanicTranslationalSpring::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicSpeedSensor", MechanicSpeedSensor::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicForceSensor", MechanicForceSensor::Creator);
    cfact_ptr->RegisterCreatorFunction("MechanicPositionSensor", MechanicPositionSensor::Creator);

}
