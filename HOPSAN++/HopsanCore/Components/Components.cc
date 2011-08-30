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

//!
//! @file   Components.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_components function that registers all built in components
//!
//$Id$

#include "Components.h"
#include "signal.h"
#include "stdlib.h"

//! @defgroup Components Components
//!
//! @defgroup HydraulicComponents HydraulicComponents
//! @ingroup Components
//!
//! @defgroup MechanicalComponents MechanicalComponents
//! @ingroup Components
//!
//! @defgroup SignalComponents SignalComponents
//! @ingroup Components

using namespace hopsan;

////Uncomment this and the Repoint SEGFAULT thing below to catch segfaults
////vvv The function to be run at SEGFAULT, see below. (http://www.cplusplus.com/reference/clibrary/csignal/signal/)
//void terminate (int /*param*/)
//{
//    std::cout << "Terminating program because of SEGFAULT, putz..." << std::endl;
//    exit(1);
//    //Maybe have a global message tracker to write ino in on whats going on that could be used to ptint here?!
//}
////^^^

//!
//! @brief Registers the creator function of all built in components
//! @param [in,out] pComponentFactory A pointer the the component factory in wich to register the components
//!
DLLIMPORTEXPORT void hopsan::register_components(ComponentFactory* pComponentFactory)
{
//    //vvv Repoint SEGFAULT to the terminate function, could be used to tell component makers info about their fault. (http://www.cplusplus.com/reference/clibrary/csignal/signal/)
//    void (*prev_fn)(int);
//    prev_fn = signal (SIGSEGV,terminate);
//    //^^^

    //Hydraulic components
    pComponentFactory->registerCreatorFunction("HydraulicLaminarOrifice", HydraulicLaminarOrifice::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTurbulentOrifice", HydraulicTurbulentOrifice::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVolume", HydraulicVolume::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureSourceC", HydraulicPressureSourceC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicMultiPressureSourceC", HydraulicMultiPressureSourceC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFlowSourceQ", HydraulicFlowSourceQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureSourceQ", HydraulicPressureSourceQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFixedDisplacementPump", HydraulicFixedDisplacementPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCheckValve", HydraulicCheckValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCheckValveWithOrifice", HydraulicCheckValveWithOrifice::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic22DirectionalValve", Hydraulic22DirectionalValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic22Valve", Hydraulic22Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic22PoppetValve", Hydraulic22PoppetValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic32DirectionalValve", Hydraulic32DirectionalValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic33Valve", Hydraulic33Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic33ShuttleValve", Hydraulic33ShuttleValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic42Valve", Hydraulic42Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43Valve", Hydraulic43Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43ValveNeutralToTank", Hydraulic43ValveNeutralToTank::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43ValveNeutralSupplyToTank", Hydraulic43ValveNeutralSupplyToTank::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43LoadSensingValve", Hydraulic43LoadSensingValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicOpenCenterValve", HydraulicOpenCenterValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVariableDisplacementPump", HydraulicVariableDisplacementPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicAckumulator", HydraulicAckumulator::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureControlledValve", HydraulicPressureControlledValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureCompensatingValve", HydraulicPressureCompensatingValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureSensor", HydraulicPressureSensor::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFlowSensor", HydraulicFlowSensor::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPowerSensor", HydraulicPowerSensor::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCylinderC", HydraulicCylinderC::Creator);
    //pComponentFactory->registerCreatorFunction("HydraulicCylinderCMulti", HydraulicCylinderCMulti::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCylinderQ", HydraulicCylinderQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTLMlossless", HydraulicTLMlossless::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureReliefValve", HydraulicPressureReliefValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureReducingValve", HydraulicPressureReducingValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureDropValve", HydraulicPressureDropValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicSubSysExample", HydraulicSubSysExample::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTankC", HydraulicTankC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTankQ", HydraulicTankQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFixedDisplacementMotorQ", HydraulicFixedDisplacementMotorQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVariableDisplacementMotorQ", HydraulicVariableDisplacementMotorQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVolume3", HydraulicVolume3::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVolume4", HydraulicVolume4::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVolumeMultiPort", HydraulicVolumeMultiPort::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicLosslessConnector", HydraulicLosslessConnector::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicLosslessTConnector", HydraulicLosslessTConnector::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicMachineC", HydraulicMachineC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicShuttleValve", HydraulicShuttleValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureControlledPump", HydraulicPressureControlledPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicHose", HydraulicHose::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicDummyC", HydraulicDummyC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicDummyQ", HydraulicDummyQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPilotControlledCheckValve", HydraulicPilotControlledCheckValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPilotClosableCheckValve", HydraulicPilotClosableCheckValve::Creator);

    pComponentFactory->registerCreatorFunction("HydraulicUndefinedConnectionC", HydraulicUndefinedConnectionC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicUndefinedConnectionQ", HydraulicUndefinedConnectionQ::Creator);


    //Signal components
    pComponentFactory->registerCreatorFunction("SignalAbsoluteValue", SignalAbsoluteValue::Creator);
    pComponentFactory->registerCreatorFunction("SignalSource", SignalSource::Creator);
    pComponentFactory->registerCreatorFunction("SignalGain", SignalGain::Creator);
    pComponentFactory->registerCreatorFunction("SignalSink", SignalSink::Creator);
    pComponentFactory->registerCreatorFunction("SignalStep", SignalStep::Creator);
    pComponentFactory->registerCreatorFunction("SignalSineWave", SignalSineWave::Creator);
    pComponentFactory->registerCreatorFunction("SignalSquareWave", SignalSquareWave::Creator);
    pComponentFactory->registerCreatorFunction("SignalRamp", SignalRamp::Creator);
    pComponentFactory->registerCreatorFunction("SignalAdd", SignalAdd::Creator);
    pComponentFactory->registerCreatorFunction("SignalSubtract", SignalSubtract::Creator);
    pComponentFactory->registerCreatorFunction("SignalMultiply", SignalMultiply::Creator);
    pComponentFactory->registerCreatorFunction("SignalDivide", SignalDivide::Creator);
    pComponentFactory->registerCreatorFunction("SignalSaturation", SignalSaturation::Creator);
    pComponentFactory->registerCreatorFunction("SignalDeadZone", SignalDeadZone::Creator);
    pComponentFactory->registerCreatorFunction("SignalLP1Filter", SignalLP1Filter::Creator);
    pComponentFactory->registerCreatorFunction("SignalLP2Filter", SignalLP2Filter::Creator);
    pComponentFactory->registerCreatorFunction("SignalHP1Filter", SignalHP1Filter::Creator);
    pComponentFactory->registerCreatorFunction("SignalHP2Filter", SignalHP2Filter::Creator);
    pComponentFactory->registerCreatorFunction("SignalPulse", SignalPulse::Creator);
    pComponentFactory->registerCreatorFunction("SignalMin", SignalMin::Creator);
    pComponentFactory->registerCreatorFunction("SignalMax", SignalMax::Creator);
    pComponentFactory->registerCreatorFunction("SignalSoftStep", SignalSoftStep::Creator);
    pComponentFactory->registerCreatorFunction("SignalRoute", SignalRoute::Creator);
    pComponentFactory->registerCreatorFunction("SignalIntegrator", SignalIntegrator::Creator);
    pComponentFactory->registerCreatorFunction("SignalIntegrator2", SignalIntegrator2::Creator);
    pComponentFactory->registerCreatorFunction("SignalIntegratorLimited", SignalIntegratorLimited::Creator);
    pComponentFactory->registerCreatorFunction("SignalIntegratorLimited2", SignalIntegratorLimited2::Creator);
    pComponentFactory->registerCreatorFunction("SignalTimeDelay", SignalTimeDelay::Creator);
    pComponentFactory->registerCreatorFunction("SignalFirstOrderFilter", SignalFirstOrderFilter::Creator);
    pComponentFactory->registerCreatorFunction("SignalSecondOrderFilter", SignalSecondOrderFilter::Creator);
    pComponentFactory->registerCreatorFunction("SignalHysteresis", SignalHysteresis::Creator);
    pComponentFactory->registerCreatorFunction("SignalSquare", SignalSquare::Creator);
    pComponentFactory->registerCreatorFunction("SignalTime", SignalTime::Creator);
    pComponentFactory->registerCreatorFunction("SignalStopSimulation", SignalStopSimulation::Creator);
    pComponentFactory->registerCreatorFunction("SignalGreaterThan", SignalGreaterThan::Creator);
    pComponentFactory->registerCreatorFunction("SignalSmallerThan", SignalSmallerThan::Creator);
    pComponentFactory->registerCreatorFunction("SignalAnd", SignalAnd::Creator);
    pComponentFactory->registerCreatorFunction("SignalOr", SignalOr::Creator);
    pComponentFactory->registerCreatorFunction("SignalXor", SignalXor::Creator);
    pComponentFactory->registerCreatorFunction("SignalSum", SignalSum::Creator);
    pComponentFactory->registerCreatorFunction("SignalSecondOrderTransferFunction", SignalSecondOrderTransferFunction::Creator);
    pComponentFactory->registerCreatorFunction("SignalFirstOrderTransferFunction", SignalFirstOrderTransferFunction::Creator);
    pComponentFactory->registerCreatorFunction("SignalPower", SignalPower::Creator);
    pComponentFactory->registerCreatorFunction("SignalDummy", SignalDummy::Creator);
    pComponentFactory->registerCreatorFunction("SignalInputInterface", SignalInputInterface::Creator);
    pComponentFactory->registerCreatorFunction("SignalOutputInterface", SignalOutputInterface::Creator);
    pComponentFactory->registerCreatorFunction("SignalNoiseGenerator", SignalNoiseGenerator::Creator);
    pComponentFactory->registerCreatorFunction("SignalAdditiveNoise", SignalAdditiveNoise::Creator);
    pComponentFactory->registerCreatorFunction("SignalStepExponentialDelay", SignalStepExponentialDelay::Creator);
    pComponentFactory->registerCreatorFunction("SignalDualRoute", SignalDualRoute::Creator);
    pComponentFactory->registerCreatorFunction("SignalTripleRoute", SignalTripleRoute::Creator);
    pComponentFactory->registerCreatorFunction("SignalQuadRoute", SignalQuadRoute::Creator);
    pComponentFactory->registerCreatorFunction("SignalBETest", SignalBETest::Creator);

    pComponentFactory->registerCreatorFunction("SignalUndefinedConnection", SignalUndefinedConnection::Creator);


    //Mechanical components
    pComponentFactory->registerCreatorFunction("MechanicForceTransformer", MechanicForceTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicVelocityTransformer", MechanicVelocityTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTorqueTransformer", MechanicTorqueTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicAngularVelocityTransformer", MechanicAngularVelocityTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalMass", MechanicTranslationalMass::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalMassWithLever", MechanicTranslationalMassWithLever::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalMassWithCoulumbFriction", MechanicTranslationalMassWithCoulumbFriction::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalSpring", MechanicTranslationalSpring::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTorsionalSpring", MechanicTorsionalSpring::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertia", MechanicRotationalInertia::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertiaWithGearRatio", MechanicRotationalInertiaWithGearRatio::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertiaWithSingleGear", MechanicRotationalInertiaWithSingleGear::Creator);
    pComponentFactory->registerCreatorFunction("MechanicSpeedSensor", MechanicSpeedSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicForceSensor", MechanicForceSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicPositionSensor", MechanicPositionSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicAngleSensor", MechanicAngleSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalLosslessConnector", MechanicTranslationalLosslessConnector::Creator);
    pComponentFactory->registerCreatorFunction("MechanicAngularVelocitySensor", MechanicAngularVelocitySensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTorqueSensor", MechanicTorqueSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicMultiPortTranslationalMass", MechanicMultiPortTranslationalMass::Creator);
    pComponentFactory->registerCreatorFunction("MechanicInterfaceC", MechanicInterfaceC::Creator);
    pComponentFactory->registerCreatorFunction("MechanicInterfaceQ", MechanicInterfaceQ::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInterfaceC", MechanicRotationalInterfaceC::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInterfaceQ", MechanicRotationalInterfaceQ::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRackAndPinion", MechanicRackAndPinion::Creator);
    pComponentFactory->registerCreatorFunction("MechanicJLink", MechanicJLink::Creator);
    pComponentFactory->registerCreatorFunction("MechanicThetaSource", MechanicThetaSource::Creator);
    pComponentFactory->registerCreatorFunction("MechanicVehicle1D", MechanicVehicle1D::Creator);

    //Electric components
    pComponentFactory->registerCreatorFunction("ElectricBattery", ElectricBattery::Creator);
    pComponentFactory->registerCreatorFunction("ElectricCapacitance2", ElectricCapacitance2::Creator);
    pComponentFactory->registerCreatorFunction("ElectricGround", ElectricGround::Creator);
    pComponentFactory->registerCreatorFunction("ElectricIcontroller", ElectricIcontroller::Creator);
    pComponentFactory->registerCreatorFunction("ElectricInductance", ElectricInductance::Creator);
    pComponentFactory->registerCreatorFunction("ElectricIsource", ElectricIsource::Creator);
    pComponentFactory->registerCreatorFunction("ElectricMotor", ElectricMotor::Creator);
    pComponentFactory->registerCreatorFunction("ElectricMotorGear", ElectricMotorGear::Creator);
    pComponentFactory->registerCreatorFunction("ElectricResistor", ElectricResistor::Creator);
    pComponentFactory->registerCreatorFunction("ElectricUsource", ElectricUsource::Creator);
}
