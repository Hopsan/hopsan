//!
//! @file   Components.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Includes all built in components
//!
//$Id$

#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include "../Component.h"
DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr);

/* Hydraulic Components */
#include "Hydraulic/HydraulicLaminarOrifice.hpp"
#include "Hydraulic/HydraulicTurbulentOrifice.hpp"
#include "Hydraulic/HydraulicVolume.hpp"
#include "Hydraulic/HydraulicPressureSource.hpp"
#include "Hydraulic/HydraulicFlowSourceQ.hpp"
#include "Hydraulic/HydraulicPressureSourceQ.hpp"
#include "Hydraulic/HydraulicFixedDisplacementPump.hpp"
#include "Hydraulic/HydraulicCheckValve.hpp"
#include "Hydraulic/Hydraulic43Valve.hpp"
#include "Hydraulic/HydraulicTurbulentOrifice.hpp"
#include "Hydraulic/HydraulicTLMRLineR.hpp"
#include "Hydraulic/HydraulicTLMlossless.hpp"
#include "Hydraulic/HydraulicVariableDisplacementPump.hpp"
#include "Hydraulic/HydraulicAckumulator.hpp"
#include "Hydraulic/HydraulicPressureControlledValve.hpp"
#include "Hydraulic/HydraulicPressureSensor.hpp"
#include "Hydraulic/HydraulicFlowSensor.hpp"
#include "Hydraulic/HydraulicPowerSensor.hpp"
#include "Hydraulic/HydraulicCylinderC.hpp"
#include "Hydraulic/HydraulicCylinderQ.hpp"
#include "Hydraulic/HydraulicPressureReliefValve.hpp"
#include "Hydraulic/HydraulicSubSysExample.hpp"
#include "Hydraulic/HydraulicTankC.hpp"


/* Signal Components */
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

/* Mechanical Components */
#include "Mechanic/MechanicForceTransformer.hpp"
#include "Mechanic/MechanicVelocityTransformer.hpp"
#include "Mechanic/MechanicTranslationalMass.hpp"
#include "Mechanic/MechanicTranslationalSpring.hpp"
#include "Mechanic/MechanicSpeedSensor.hpp"
#include "Mechanic/MechanicForceSensor.hpp"
#include "Mechanic/MechanicPositionSensor.hpp"

#endif // COMPONENTS_H_INCLUDED
