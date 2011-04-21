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
#include "Hydraulic/HydraulicLaminarOrifice.hpp"
#include "Hydraulic/HydraulicTurbulentOrifice.hpp"
#include "Hydraulic/HydraulicVolume.hpp"
#include "Hydraulic/HydraulicPressureSourceC.hpp"
#include "Hydraulic/HydraulicMultiPressureSourceC.hpp"
#include "Hydraulic/HydraulicFlowSourceQ.hpp"
#include "Hydraulic/HydraulicPressureSourceQ.hpp"
#include "Hydraulic/HydraulicFixedDisplacementPump.hpp"
#include "Hydraulic/HydraulicCheckValve.hpp"
#include "Hydraulic/Hydraulic22DirectionalValve.hpp"
#include "Hydraulic/Hydraulic22Valve.hpp"
#include "Hydraulic/Hydraulic32DirectionalValve.hpp"
#include "Hydraulic/Hydraulic33Valve.hpp"
#include "Hydraulic/Hydraulic42Valve.hpp"
#include "Hydraulic/Hydraulic43Valve.hpp"
#include "Hydraulic/Hydraulic43LoadSensingValve.hpp"
#include "Hydraulic/HydraulicOpenCenterValve.hpp"
#include "Hydraulic/HydraulicTurbulentOrifice.hpp"
#include "Hydraulic/HydraulicTLMRLineR.hpp"
#include "Hydraulic/HydraulicTLMlossless.hpp"
#include "Hydraulic/HydraulicVariableDisplacementPump.hpp"
#include "Hydraulic/HydraulicAckumulator.hpp"
#include "Hydraulic/HydraulicPressureControlledValve.hpp"
#include "Hydraulic/HydraulicPressureCompensatingValve.hpp"
#include "Hydraulic/HydraulicPressureSensor.hpp"
#include "Hydraulic/HydraulicFlowSensor.hpp"
#include "Hydraulic/HydraulicPowerSensor.hpp"
#include "Hydraulic/HydraulicCylinderC.hpp"
#include "Hydraulic/HydraulicCylinderQ.hpp"
#include "Hydraulic/HydraulicPressureReliefValve.hpp"
#include "Hydraulic/HydraulicPressureReducingValve.hpp"
#include "Hydraulic/HydraulicPressureDropValve.hpp"
#include "Hydraulic/HydraulicSubSysExample.hpp"
#include "Hydraulic/HydraulicTankC.hpp"
#include "Hydraulic/HydraulicTankQ.hpp"
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
#include "Hydraulic/HydraulicShuttleValve.hpp"
#include "Hydraulic/HydraulicPressureControlledPump.hpp"
#include "Hydraulic/HydraulicUndefinedConnectionC.hpp"
#include "Hydraulic/HydraulicUndefinedConnectionQ.hpp"


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
#include "Signal/SignalMin.hpp"
#include "Signal/SignalMax.hpp"
#include "Signal/SignalPulse.hpp"
#include "Signal/SignalSoftStep.hpp"
#include "Signal/SignalIntegrator.hpp"
#include "Signal/SignalIntegrator2.hpp"
#include "Signal/SignalIntegratorLimited.hpp"
#include "Signal/SignalIntegratorLimited2.hpp"
#include "Signal/SignalTimeDelay.hpp"
#include "Signal/SignalFirstOrderFilter.hpp"
#include "Signal/SignalSecondOrderFilter.hpp"
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



#include "../Component.h"

namespace hopsan {

    DLLIMPORTEXPORT void register_components(ComponentFactory* cfampND_ct);
}

#endif // COMPONENTS_H_INCLUDED
