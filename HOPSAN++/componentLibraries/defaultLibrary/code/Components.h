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
//! @file   Components.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Includes all built in components
//!
//$Id$

#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

/* Hydraulic Components */
#include "Hydraulic/valves/HydraulicValves.h"

#include "Hydraulic/HydraulicLaminarOrifice.hpp"
#include "Hydraulic/HydraulicTurbulentOrifice.hpp"
#include "Hydraulic/HydraulicVolume.hpp"
#include "Hydraulic/HydraulicPressureSourceC.hpp"
#include "Hydraulic/HydraulicMultiPressureSourceC.hpp"
#include "Hydraulic/HydraulicFlowSourceQ.hpp"
#include "Hydraulic/HydraulicPressureSourceQ.hpp"
#include "Hydraulic/HydraulicFixedDisplacementPump.hpp"
#include "Hydraulic/HydraulicOpenCenterValve.hpp"
#include "Hydraulic/HydraulicTurbulentOrifice.hpp"
#include "Hydraulic/HydraulicTLMRLineR.hpp"
#include "Hydraulic/HydraulicTLMlossless.hpp"
#include "Hydraulic/HydraulicVariableDisplacementPump.hpp"
#include "Hydraulic/HydraulicAckumulator.hpp"

#include "Hydraulic/HydraulicPressureSensor.hpp"
#include "Hydraulic/HydraulicFlowSensor.hpp"
#include "Hydraulic/HydraulicPowerSensor.hpp"
#include "Hydraulic/HydraulicCylinderC.hpp"
//#include "Hydraulic/HydraulicCylinderCMultiports.hpp"
#include "Hydraulic/HydraulicCylinderQ.hpp"
//#include "Hydraulic/HydraulicPistonMload.hpp"

#include "Hydraulic/HydraulicSubSysExample.hpp"
#include "Hydraulic/HydraulicTankC.hpp"
#include "Hydraulic/HydraulicMultiTankC.hpp"
//#include "Hydraulic/HydraulicTankQ.hpp"
#include "Hydraulic/HydraulicFixedDisplacementMotorQ.hpp"
#include "Hydraulic/HydraulicVariableDisplacementMotorQ.hpp"
#include "Hydraulic/HydraulicVolume3.hpp"
#include "Hydraulic/HydraulicVolume4.hpp"
#include "Hydraulic/HydraulicVolumeMultiPort.hpp"
#include "Hydraulic/HydraulicLossLessConnector.hpp"
#include "Hydraulic/HydraulicLossLessTConnector.hpp"
#include "Hydraulic/HydraulicDummyC.hpp"
#include "Hydraulic/HydraulicDummyQ.hpp"
#include "Hydraulic/HydraulicMachineC.hpp"

#include "Hydraulic/HydraulicPressureControlledPump.hpp"
#include "Hydraulic/HydraulicHose.hpp"
#include "Hydraulic/HydraulicUndefinedConnectionC.hpp"
#include "Hydraulic/HydraulicUndefinedConnectionQ.hpp"
#include "Hydraulic/HydraulicPilotControlledCheckValve.hpp"
#include "Hydraulic/HydraulicPilotClosableCheckValve.hpp"
#include "Hydraulic/HydraulicOverCenterValve.hpp"
#include "Hydraulic/HydraulicInterfaceC.hpp"
#include "Hydraulic/HydraulicInterfaceQ.hpp"

#include "Hydraulic/HydraulicComponentsInComponentTest.hpp"

//#include "Hydraulic/HydraulicPressureReliefValveTolerance.hpp"
//#include "Hydraulic/HydraulicCylinderCTolerance.hpp"
//#include "Hydraulic/HydraulicHoseTolerance.hpp"


/* Signal Components */
#include "Signal/SignalAbsoluteValue.hpp"
#include "Signal/SignalSource.hpp"
#include "Signal/SignalGain.hpp"
#include "Signal/SignalSink.hpp"
#include "Signal/SignalStep.hpp"
#include "Signal/SignalSineWave.hpp"
#include "Signal/SignalSquareWave.hpp"
#include "Signal/SignalRamp.hpp"
#include "Signal/SignalAdd.hpp"
#include "Signal/SignalSubtract.hpp"
#include "Signal/SignalMultiply.hpp"
#include "Signal/SignalDivide.hpp"
#include "Signal/SignalSaturation.hpp"
#include "Signal/SignalDeadZone.hpp"
#include "Signal/SignalLP1Filter.hpp"
#include "Signal/SignalLP2Filter.hpp"
#include "Signal/SignalHP1Filter.hpp"
#include "Signal/SignalHP2Filter.hpp"
#include "Signal/SignalPIlead.hpp"
#include "Signal/SignalMin.hpp"
#include "Signal/SignalMax.hpp"
#include "Signal/SignalPulse.hpp"
#include "Signal/SignalRoute.hpp"
#include "Signal/SignalSoftStep.hpp"
#include "Signal/SignalIntegrator.hpp"
#include "Signal/SignalIntegrator2.hpp"
#include "Signal/SignalIntegratorLimited.hpp"
#include "Signal/SignalIntegratorLimited2.hpp"
#include "Signal/SignalTimeDelay.hpp"
#include "Signal/SignalFirstOrderFilter.hpp"
#include "Signal/SignalFirstOrderTransferFunction.hpp"
#include "Signal/SignalSecondOrderFilter.hpp"
#include "Signal/SignalSecondOrderTransferFunction.hpp"
#include "Signal/SignalPIlead.hpp"
#include "Signal/SignalHysteresis.hpp"
#include "Signal/SignalSquare.hpp"
#include "Signal/SignalTime.hpp"
#include "Signal/SignalStopSimulation.hpp"
#include "Signal/SignalGreaterThan.hpp"
#include "Signal/SignalSmallerThan.hpp"
#include "Signal/SignalAnd.hpp"
#include "Signal/SignalOr.hpp"
#include "Signal/SignalXor.hpp"
#include "Signal/SignalSum.hpp"
#include "Signal/SignalDummy.hpp"
#include "Signal/SignalPower.hpp"
#include "Signal/SignalInputInterface.hpp"
#include "Signal/SignalOutputInterface.hpp"
#include "Signal/SignalUndefinedConnection.hpp"
#include "Signal/SignalNoiseGenerator.hpp"
#include "Signal/SignalAdditiveNoise.hpp"
#include "Signal/SignalStepExponentialDelay.hpp"
#include "Signal/SignalDualRoute.hpp"
#include "Signal/SignalTripleRoute.hpp"
#include "Signal/SignalQuadRoute.hpp"
#include "Signal/SignalBETest.hpp"
#include "Signal/SignalSin.hpp"
#include "Signal/SignalCos.hpp"
#include "Signal/SignalTan.hpp"
#include "Signal/SignalLookUpTable2D.hpp"

/* Mechanical Components */
#include "Mechanic/MechanicForceTransformer.hpp"
#include "Mechanic/MechanicVelocityTransformer.hpp"
#include "Mechanic/MechanicAngularVelocityTransformer.hpp"
#include "Mechanic/MechanicTorqueTransformer.hpp"
#include "Mechanic/MechanicTranslationalMass.hpp"
#include "Mechanic/MechanicTranslationalMassWithLever.hpp"
#include "Mechanic/MechanicTranslationalMassWithCoulumbFriction.hpp"
#include "Mechanic/MechanicTranslationalSpring.hpp"
#include "Mechanic/MechanicRotationalInertia.hpp"
#include "Mechanic/MechanicRotationalInertiaWithGearRatio.hpp"
#include "Mechanic/MechanicRotationalInertiaWithSingleGear.hpp"
#include "Mechanic/MechanicTorsionalSpring.hpp"
#include "Mechanic/MechanicSpeedSensor.hpp"
#include "Mechanic/MechanicForceSensor.hpp"
#include "Mechanic/MechanicPositionSensor.hpp"
#include "Mechanic/MechanicAngleSensor.hpp"
#include "Mechanic/MechanicTranslationalLosslessConnector.hpp"
#include "Mechanic/MechanicAngularVelocitySensor.hpp"
#include "Mechanic/MechanicTorqueSensor.hpp"
#include "Mechanic/MechanicMultiPortTranslationalMass.hpp"
#include "Mechanic/MechanicInterfaceC.hpp"
#include "Mechanic/MechanicInterfaceQ.hpp"
#include "Mechanic/MechanicRotationalInterfaceC.hpp"
#include "Mechanic/MechanicRotationalInterfaceQ.hpp"
#include "Mechanic/MechanicRackAndPinion.hpp"
#include "Mechanic/MechanicJLink.hpp"
#include "Mechanic/MechanicThetaSource.hpp"
#include "Mechanic/MechanicVehicle1D.hpp"
#include "Mechanic/MechanicRotationalInertiaWithCoulumbFriction.hpp"
#include "Mechanic/MechanicFixedPosition.hpp"
#include "Mechanic/MechanicFreeLengthWall.hpp"

/* Electric Components */
#include "Electric/ElectricBattery.hpp"
#include "Electric/ElectricCapacitance2.hpp"
#include "Electric/ElectricGround.hpp"
#include "Electric/ElectricMotor.hpp"
#include "Electric/ElectricMotorGear.hpp"
#include "Electric/ElectricIcontroller.hpp"
#include "Electric/ElectricInductance.hpp"
#include "Electric/ElectricIsource.hpp"
#include "Electric/ElectricResistor.hpp"
#include "Electric/ElectricVarResistor.hpp"
#include "Electric/ElectricSwitch.hpp"
#include "Electric/ElectricUsource.hpp"

#endif // COMPONENTS_H_INCLUDED
