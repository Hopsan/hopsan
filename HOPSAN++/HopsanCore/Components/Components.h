#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include "../Component.h"
DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr);

/* Hydraulic Components */
#include "Hydraulic/HydraulicLaminarOrifice.hpp"
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

#endif // COMPONENTS_H_INCLUDED
