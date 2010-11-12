//!
//! @file   Components.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_components function that registers all built in components
//!
//$Id$

#include "Components.h"

//! @defgroup HydraulicComponents HydraulicComponents
//! @ingroup Components
//!
//! @defgroup MechanicalComponents MechanicalComponents
//! @ingroup Components
//!
//! @defgroup SignalComponents SignalComponents
//! @ingroup Components

//!
//! @brief Registers the creator function of all built in components
//! @param [in,out] cfact_ptr A pointer the the component factory in wich to register the components
//!

using namespace hopsan;

DLLIMPORTEXPORT void hopsan::register_components(ComponentFactory* cfact_ptr)
{
    //Hydraulic components
    cfact_ptr->registerCreatorFunction("HydraulicLaminarOrifice", HydraulicLaminarOrifice::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicTurbulentOrifice", HydraulicTurbulentOrifice::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicVolume", HydraulicVolume::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicPressureSource", HydraulicPressureSource::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicFlowSourceQ", HydraulicFlowSourceQ::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicPressureSourceQ", HydraulicPressureSourceQ::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicFixedDisplacementPump", HydraulicFixedDisplacementPump::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicCheckValve", HydraulicCheckValve::Creator);
    cfact_ptr->registerCreatorFunction("Hydraulic43Valve", Hydraulic43Valve::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicVariableDisplacementPump", HydraulicVariableDisplacementPump::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicAckumulator", HydraulicAckumulator::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicPressureControlledValve", HydraulicPressureControlledValve::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicPressureSensor", HydraulicPressureSensor::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicFlowSensor", HydraulicFlowSensor::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicPowerSensor", HydraulicPowerSensor::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicCylinderC", HydraulicCylinderC::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicCylinderQ", HydraulicCylinderQ::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicTLMlossless", HydraulicTLMlossless::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicPressureReliefValve", HydraulicPressureReliefValve::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicSubSysExample", HydraulicSubSysExample::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicTankC", HydraulicTankC::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicAlternativePRV", HydraulicAlternativePRV::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicFixedDisplacementMotorQ", HydraulicFixedDisplacementMotorQ::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicVariableDisplacementMotorQ", HydraulicVariableDisplacementMotorQ::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicVolume3", HydraulicVolume3::Creator);

    cfact_ptr->registerCreatorFunction("HydraulicOptimizedCylinderQ", HydraulicOptimizedCylinderQ::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicOptimized43Valve", HydraulicOptimized43Valve::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicOptimizedVolume", HydraulicOptimizedVolume::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicOptimizedTurbulentOrifice", HydraulicOptimizedTurbulentOrifice::Creator);
    cfact_ptr->registerCreatorFunction("HydraulicOptimizedPressureSource", HydraulicOptimizedPressureSource::Creator);



    //Signal components
    cfact_ptr->registerCreatorFunction("SignalSource", SignalSource::Creator);
    cfact_ptr->registerCreatorFunction("SignalGain", SignalGain::Creator);
    cfact_ptr->registerCreatorFunction("SignalSink", SignalSink::Creator);
    cfact_ptr->registerCreatorFunction("SignalStep", SignalStep::Creator);
    cfact_ptr->registerCreatorFunction("SignalSineWave", SignalSineWave::Creator);
    cfact_ptr->registerCreatorFunction("SignalSquareWave", SignalSquareWave::Creator);
    cfact_ptr->registerCreatorFunction("SignalRamp", SignalRamp::Creator);
    cfact_ptr->registerCreatorFunction("SignalAdd", SignalAdd::Creator);
    cfact_ptr->registerCreatorFunction("SignalSubtract", SignalSubtract::Creator);
    cfact_ptr->registerCreatorFunction("SignalMultiply", SignalMultiply::Creator);
    cfact_ptr->registerCreatorFunction("SignalDivide", SignalDivide::Creator);
    cfact_ptr->registerCreatorFunction("SignalSaturation", SignalSaturation::Creator);
    cfact_ptr->registerCreatorFunction("SignalDeadZone", SignalDeadZone::Creator);
    cfact_ptr->registerCreatorFunction("SignalLP1Filter", SignalLP1Filter::Creator);
    cfact_ptr->registerCreatorFunction("SignalLP2Filter", SignalLP2Filter::Creator);
    cfact_ptr->registerCreatorFunction("SignalPulse", SignalPulse::Creator);
    cfact_ptr->registerCreatorFunction("SignalSoftStep", SignalSoftStep::Creator);
    cfact_ptr->registerCreatorFunction("SignalIntegrator", SignalIntegrator::Creator);
    cfact_ptr->registerCreatorFunction("SignalIntegrator2", SignalIntegrator2::Creator);
    cfact_ptr->registerCreatorFunction("SignalIntegratorLimited", SignalIntegratorLimited::Creator);
    cfact_ptr->registerCreatorFunction("SignalIntegratorLimited2", SignalIntegratorLimited2::Creator);
    cfact_ptr->registerCreatorFunction("SignalTimeDelay", SignalTimeDelay::Creator);
    cfact_ptr->registerCreatorFunction("SignalFirstOrderFilter", SignalFirstOrderFilter::Creator);
    cfact_ptr->registerCreatorFunction("SignalSecondOrderFilter", SignalSecondOrderFilter::Creator);
    cfact_ptr->registerCreatorFunction("SignalHysteresis", SignalHysteresis::Creator);
    cfact_ptr->registerCreatorFunction("SignalSquare", SignalSquare::Creator);
    cfact_ptr->registerCreatorFunction("SignalTime", SignalTime::Creator);
    cfact_ptr->registerCreatorFunction("SignalStopSimulation", SignalStopSimulation::Creator);
    cfact_ptr->registerCreatorFunction("SignalGreaterThan", SignalGreaterThan::Creator);
    cfact_ptr->registerCreatorFunction("SignalSmallerThan", SignalSmallerThan::Creator);
    cfact_ptr->registerCreatorFunction("SignalAnd", SignalAnd::Creator);
    cfact_ptr->registerCreatorFunction("SignalOr", SignalOr::Creator);
    cfact_ptr->registerCreatorFunction("SignalXor", SignalXor::Creator);

    cfact_ptr->registerCreatorFunction("SignalOptimizedSineWave", SignalOptimizedSineWave::Creator);
    cfact_ptr->registerCreatorFunction("SignalOptimizedSubtract", SignalOptimizedSubtract::Creator);
    cfact_ptr->registerCreatorFunction("SignalOptimizedGain", SignalOptimizedGain::Creator);
    cfact_ptr->registerCreatorFunction("SignalOptimizedSource", SignalOptimizedSource::Creator);
    cfact_ptr->registerCreatorFunction("SignalDummy", SignalDummy::Creator);



    //Mechanical components
    cfact_ptr->registerCreatorFunction("MechanicForceTransformer", MechanicForceTransformer::Creator);
    cfact_ptr->registerCreatorFunction("MechanicVelocityTransformer", MechanicVelocityTransformer::Creator);
    cfact_ptr->registerCreatorFunction("MechanicTorqueTransformer", MechanicTorqueTransformer::Creator);
    cfact_ptr->registerCreatorFunction("MechanicAngularVelocityTransformer", MechanicAngularVelocityTransformer::Creator);
    cfact_ptr->registerCreatorFunction("MechanicTranslationalMass", MechanicTranslationalMass::Creator);
    cfact_ptr->registerCreatorFunction("MechanicTranslationalSpring", MechanicTranslationalSpring::Creator);
    cfact_ptr->registerCreatorFunction("MechanicTorsionalSpring", MechanicTorsionalSpring::Creator);
    cfact_ptr->registerCreatorFunction("MechanicRotationalInertia", MechanicRotationalInertia::Creator);
    cfact_ptr->registerCreatorFunction("MechanicSpeedSensor", MechanicSpeedSensor::Creator);
    cfact_ptr->registerCreatorFunction("MechanicForceSensor", MechanicForceSensor::Creator);
    cfact_ptr->registerCreatorFunction("MechanicPositionSensor", MechanicPositionSensor::Creator);
    cfact_ptr->registerCreatorFunction("MechanicAngleSensor", MechanicAngleSensor::Creator);

    cfact_ptr->registerCreatorFunction("MechanicOptimizedPositionSensor", MechanicOptimizedPositionSensor::Creator);
    cfact_ptr->registerCreatorFunction("MechanicOptimizedForceTransformer", MechanicOptimizedForceTransformer::Creator);




}
