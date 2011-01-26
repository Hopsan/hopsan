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
//! @param [in,out] cfampND_ct A pointer the the component factory in wich to register the components
//!

using namespace hopsan;

DLLIMPORTEXPORT void hopsan::register_components(ComponentFactory* cfampND_ct)
{
    //Hydraulic components
    cfampND_ct->registerCreatorFunction("HydraulicLaminarOrifice", HydraulicLaminarOrifice::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicTurbulentOrifice", HydraulicTurbulentOrifice::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicVolume", HydraulicVolume::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureSource", HydraulicPressureSource::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicFlowSourceQ", HydraulicFlowSourceQ::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureSourceQ", HydraulicPressureSourceQ::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicFixedDisplacementPump", HydraulicFixedDisplacementPump::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicCheckValve", HydraulicCheckValve::Creator);
    cfampND_ct->registerCreatorFunction("Hydraulic22Valve", Hydraulic22Valve::Creator);
    cfampND_ct->registerCreatorFunction("Hydraulic32DirectionalValve", Hydraulic32DirectionalValve::Creator);
    cfampND_ct->registerCreatorFunction("Hydraulic33Valve", Hydraulic33Valve::Creator);
    cfampND_ct->registerCreatorFunction("Hydraulic42Valve", Hydraulic42Valve::Creator);
    cfampND_ct->registerCreatorFunction("Hydraulic43Valve", Hydraulic43Valve::Creator);
    cfampND_ct->registerCreatorFunction("Hydraulic43LoadSensingValve", Hydraulic43LoadSensingValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicOpenCenterValve", HydraulicOpenCenterValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicVariableDisplacementPump", HydraulicVariableDisplacementPump::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicAckumulator", HydraulicAckumulator::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureControlledValve", HydraulicPressureControlledValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureSensor", HydraulicPressureSensor::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicFlowSensor", HydraulicFlowSensor::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPowerSensor", HydraulicPowerSensor::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicCylinderC", HydraulicCylinderC::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicCylinderQ", HydraulicCylinderQ::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicTLMlossless", HydraulicTLMlossless::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureReliefValve", HydraulicPressureReliefValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureReducingValve", HydraulicPressureReducingValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureDropValve", HydraulicPressureDropValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicSubSysExample", HydraulicSubSysExample::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicTankC", HydraulicTankC::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicFixedDisplacementMotorQ", HydraulicFixedDisplacementMotorQ::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicVariableDisplacementMotorQ", HydraulicVariableDisplacementMotorQ::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicVolume3", HydraulicVolume3::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicVolume4", HydraulicVolume4::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicLosslessConnector", HydraulicLosslessConnector::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicLosslessTConnector", HydraulicLosslessTConnector::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicMachineC", HydraulicMachineC::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicShuttleValve", HydraulicShuttleValve::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicPressureControlledPump", HydraulicPressureControlledPump::Creator);

    cfampND_ct->registerCreatorFunction("HydraulicDummyC", HydraulicDummyC::Creator);
    cfampND_ct->registerCreatorFunction("HydraulicDummyQ", HydraulicDummyQ::Creator);


    //Signal components
    cfampND_ct->registerCreatorFunction("SignalSource", SignalSource::Creator);
    cfampND_ct->registerCreatorFunction("SignalGain", SignalGain::Creator);
    cfampND_ct->registerCreatorFunction("SignalSink", SignalSink::Creator);
    cfampND_ct->registerCreatorFunction("SignalStep", SignalStep::Creator);
    cfampND_ct->registerCreatorFunction("SignalSineWave", SignalSineWave::Creator);
    cfampND_ct->registerCreatorFunction("SignalSquareWave", SignalSquareWave::Creator);
    cfampND_ct->registerCreatorFunction("SignalRamp", SignalRamp::Creator);
    cfampND_ct->registerCreatorFunction("SignalAdd", SignalAdd::Creator);
    cfampND_ct->registerCreatorFunction("SignalSubtract", SignalSubtract::Creator);
    cfampND_ct->registerCreatorFunction("SignalMultiply", SignalMultiply::Creator);
    cfampND_ct->registerCreatorFunction("SignalDivide", SignalDivide::Creator);
    cfampND_ct->registerCreatorFunction("SignalSaturation", SignalSaturation::Creator);
    cfampND_ct->registerCreatorFunction("SignalDeadZone", SignalDeadZone::Creator);
    cfampND_ct->registerCreatorFunction("SignalLP1Filter", SignalLP1Filter::Creator);
    cfampND_ct->registerCreatorFunction("SignalLP2Filter", SignalLP2Filter::Creator);
    cfampND_ct->registerCreatorFunction("SignalHP1Filter", SignalHP1Filter::Creator);
    cfampND_ct->registerCreatorFunction("SignalHP2Filter", SignalHP2Filter::Creator);
    cfampND_ct->registerCreatorFunction("SignalPulse", SignalPulse::Creator);
    cfampND_ct->registerCreatorFunction("SignalSoftStep", SignalSoftStep::Creator);
    cfampND_ct->registerCreatorFunction("SignalIntegrator", SignalIntegrator::Creator);
    cfampND_ct->registerCreatorFunction("SignalIntegrator2", SignalIntegrator2::Creator);
    cfampND_ct->registerCreatorFunction("SignalIntegratorLimited", SignalIntegratorLimited::Creator);
    cfampND_ct->registerCreatorFunction("SignalIntegratorLimited2", SignalIntegratorLimited2::Creator);
    cfampND_ct->registerCreatorFunction("SignalTimeDelay", SignalTimeDelay::Creator);
    cfampND_ct->registerCreatorFunction("SignalFirstOrderFilter", SignalFirstOrderFilter::Creator);
    cfampND_ct->registerCreatorFunction("SignalSecondOrderFilter", SignalSecondOrderFilter::Creator);
    cfampND_ct->registerCreatorFunction("SignalHysteresis", SignalHysteresis::Creator);
    cfampND_ct->registerCreatorFunction("SignalSquare", SignalSquare::Creator);
    cfampND_ct->registerCreatorFunction("SignalTime", SignalTime::Creator);
    cfampND_ct->registerCreatorFunction("SignalStopSimulation", SignalStopSimulation::Creator);
    cfampND_ct->registerCreatorFunction("SignalGreaterThan", SignalGreaterThan::Creator);
    cfampND_ct->registerCreatorFunction("SignalSmallerThan", SignalSmallerThan::Creator);
    cfampND_ct->registerCreatorFunction("SignalAnd", SignalAnd::Creator);
    cfampND_ct->registerCreatorFunction("SignalOr", SignalOr::Creator);
    cfampND_ct->registerCreatorFunction("SignalXor", SignalXor::Creator);

    cfampND_ct->registerCreatorFunction("SignalDummy", SignalDummy::Creator);


    //Mechanical components
    cfampND_ct->registerCreatorFunction("MechanicForceTransformer", MechanicForceTransformer::Creator);
    cfampND_ct->registerCreatorFunction("MechanicVelocityTransformer", MechanicVelocityTransformer::Creator);
    cfampND_ct->registerCreatorFunction("MechanicTorqueTransformer", MechanicTorqueTransformer::Creator);
    cfampND_ct->registerCreatorFunction("MechanicAngularVelocityTransformer", MechanicAngularVelocityTransformer::Creator);
    cfampND_ct->registerCreatorFunction("MechanicTranslationalMass", MechanicTranslationalMass::Creator);
    cfampND_ct->registerCreatorFunction("MechanicTranslationalMassWithCoulumbFriction", MechanicTranslationalMassWithCoulumbFriction::Creator);
    cfampND_ct->registerCreatorFunction("MechanicTranslationalSpring", MechanicTranslationalSpring::Creator);
    cfampND_ct->registerCreatorFunction("MechanicTorsionalSpring", MechanicTorsionalSpring::Creator);
    cfampND_ct->registerCreatorFunction("MechanicRotationalInertia", MechanicRotationalInertia::Creator);
    cfampND_ct->registerCreatorFunction("MechanicSpeedSensor", MechanicSpeedSensor::Creator);
    cfampND_ct->registerCreatorFunction("MechanicForceSensor", MechanicForceSensor::Creator);
    cfampND_ct->registerCreatorFunction("MechanicPositionSensor", MechanicPositionSensor::Creator);
    cfampND_ct->registerCreatorFunction("MechanicAngleSensor", MechanicAngleSensor::Creator);
    cfampND_ct->registerCreatorFunction("MechanicTranslationalLosslessConnector", MechanicTranslationalLosslessConnector::Creator);
}
