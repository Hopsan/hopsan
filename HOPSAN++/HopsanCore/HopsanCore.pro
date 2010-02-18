# -------------------------------------------------
# Global project options
# -------------------------------------------------
QT -= core gui
TARGET = HopsanCore
TEMPLATE = lib
CONFIG += dll
DESTDIR = ../bin/debug

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------

# win32:DEFINES += STATICCORE
win32:DEFINES += DOCOREDLLEXPORT
win32:DEFINES -= UNICODE


# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += Port.cc \
    Node.cc \
    Component.cc \
    Nodes/Nodes.cc \
    CoreUtilities/ValveHysteresis.cc \
    CoreUtilities/TurbulentFlowFunction.cc \
    CoreUtilities/TransferFunction.cc \
    CoreUtilities/SecondOrderFilter.cc \
    CoreUtilities/LoadExternal.cc \
    CoreUtilities/IntegratorLimited.cc \
    CoreUtilities/Integrator.cc \
    CoreUtilities/FirstOrderFilter.cc \
    CoreUtilities/FileAccess.cc \
    CoreUtilities/Delay.cc \
    Components/Components.cc
HEADERS += win32dll.h \
    Port.h \
    Node.h \
    HopsanCore.h \
    Component.h \
    Nodes/Nodes.h \
    CoreUtilities/ValveHysteresis.h \
    CoreUtilities/TurbulentFlowFunction.h \
    CoreUtilities/TransferFunction.h \
    CoreUtilities/SecondOrderFilter.h \
    CoreUtilities/LoadExternal.h \
    CoreUtilities/IntegratorLimited.h \
    CoreUtilities/Integrator.h \
    CoreUtilities/FirstOrderFilter.h \
    CoreUtilities/FileAccess.h \
    CoreUtilities/Delay.h \
    CoreUtilities/ClassFactory.h \
    Components/HydraulicComponentTemplate.hpp \
    Components/Components.h \
    Components/Signal/SignalTimeDelay.hpp \
    Components/Signal/SignalSubtract.hpp \
    Components/Signal/SignalStep.hpp \
    Components/Signal/SignalSquareWave.hpp \
    Components/Signal/SignalSource.hpp \
    Components/Signal/SignalSoftStep.hpp \
    Components/Signal/SignalSink.hpp \
    Components/Signal/SignalSineWave.hpp \
    Components/Signal/SignalSecondOrderFilter.hpp \
    Components/Signal/SignalSaturation.hpp \
    Components/Signal/SignalRamp.hpp \
    Components/Signal/SignalPulse.hpp \
    Components/Signal/SignalMultiply.hpp \
    Components/Signal/SignalLP2Filter.hpp \
    Components/Signal/SignalLP1Filter.hpp \
    Components/Signal/SignalIntegratorLimited2.hpp \
    Components/Signal/SignalIntegratorLimited.hpp \
    Components/Signal/SignalIntegrator2.hpp \
    Components/Signal/SignalIntegrator.hpp \
    Components/Signal/SignalHysteresis.hpp \
    Components/Signal/SignalGain.hpp \
    Components/Signal/SignalFirstOrderFilter.hpp \
    Components/Signal/SignalDivide.hpp \
    Components/Signal/SignalDeadZone.hpp \
    Components/Signal/SignalAdd.hpp \
    Components/Mechanic/MechanicVelocityTransformer.hpp \
    Components/Mechanic/MechanicTranslationalSpring.hpp \
    Components/Mechanic/MechanicTranslationalMass.hpp \
    Components/Mechanic/MechanicSpeedSensor.hpp \
    Components/Mechanic/MechanicPositionSensor.hpp \
    Components/Mechanic/MechanicForceTransformer.hpp \
    Components/Mechanic/MechanicForceSensor.hpp \
    Components/Hydraulic/HydraulicVolume.hpp \
    Components/Hydraulic/HydraulicVariableDisplacementPump.hpp \
    Components/Hydraulic/HydraulicTurbulentOrifice.hpp \
    Components/Hydraulic/HydraulicTLMRLineR.hpp \
    Components/Hydraulic/HydraulicTLMlossless.hpp \
    Components/Hydraulic/HydraulicSubSysExample.hpp \
    Components/Hydraulic/HydraulicPressureSourceQ.hpp \
    Components/Hydraulic/HydraulicPressureSource.hpp \
    Components/Hydraulic/HydraulicPressureSensor.hpp \
    Components/Hydraulic/HydraulicPressureReliefValve.hpp \
    Components/Hydraulic/HydraulicPressureControlledValve.hpp \
    Components/Hydraulic/HydraulicPowerSensor.hpp \
    Components/Hydraulic/HydraulicLaminarOrifice.hpp \
    Components/Hydraulic/HydraulicFlowSourceQ.hpp \
    Components/Hydraulic/HydraulicFlowSensor.hpp \
    Components/Hydraulic/HydraulicFixedDisplacementPump.hpp \
    Components/Hydraulic/HydraulicCylinderQ.hpp \
    Components/Hydraulic/HydraulicCylinderC.hpp \
    Components/Hydraulic/HydraulicCheckValve.hpp \
    Components/Hydraulic/HydraulicAckumulator.hpp \
    Components/Hydraulic/Hydraulic43Valve.hpp \
    ComponentEssentials.h \
    ComponentUtilities.h
