# -------------------------------------------------
# Global project options
# -------------------------------------------------
QT -= core \
    gui
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
#win32:INCLUDEPATH += c:\tbb30_018oss\include
#win32:INCLUDEPATH += c:\tbb\tbb30_20100406oss\include
#win32:INCLUDEPATH += c:\tbb\tbb22_20090809oss\include
win32:INCLUDEPATH += c:\tbb\tbb30_056oss\include
winr32:LIBS += c:\tbb\tbb30_056oss\build\windows_ia32_gcc_mingw_debug

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += Port.cc \
    Node.cc \
    Component.cc \
    Nodes/Nodes.cc \
    CoreUtilities/LoadExternal.cc \
    CoreUtilities/FileAccess.cc \
    Components/Components.cc \
    HopsanEssentials.cc \
    ComponentUtilities/ValveHysteresis.cc \
    ComponentUtilities/TurbulentFlowFunction.cc \
    ComponentUtilities/TransferFunction.cc \
    ComponentUtilities/SecondOrderFilter.cc \
    ComponentUtilities/IntegratorLimited.cc \
    ComponentUtilities/Integrator.cc \
    ComponentUtilities/FirstOrderFilter.cc \
    ComponentUtilities/Delay.cc \
    CoreUtilities/HopsanCoreMessageHandler.cc \
    ComponentUtilities/AuxiliarySimulationFunctions.cpp \
    ComponentUtilities/DoubleIntegratorWithDamping.cpp
HEADERS += win32dll.h \
    Port.h \
    Node.h \
    HopsanCore.h \
    Component.h \
    Nodes/Nodes.h \
    CoreUtilities/LoadExternal.h \
    CoreUtilities/FileAccess.h \
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
    ComponentUtilities.h \
    HopsanEssentials.h \
    ComponentUtilities/ValveHysteresis.h \
    ComponentUtilities/TurbulentFlowFunction.h \
    ComponentUtilities/TransferFunction.h \
    ComponentUtilities/SecondOrderFilter.h \
    ComponentUtilities/IntegratorLimited.h \
    ComponentUtilities/Integrator.h \
    ComponentUtilities/FirstOrderFilter.h \
    ComponentUtilities/Delay.h \
    CoreUtilities/HopsanCoreMessageHandler.h \
    version.h \
    Components/Hydraulic/HydraulicTankC.hpp \
    Components/Hydraulic/HydraulicAlternativePRV.hpp \
    ComponentUtilities/AuxiliarySimulationFunctions.h \
    Components/Hydraulic/HydraulicPressureControlledPump.hpp \
    ComponentUtilities/DoubleIntegratorWithDamping.h \
    Components/Hydraulic/HydraulicFixedDisplacementMotorQ.hpp \
    Components/Hydraulic/HydraulicVariableDisplacementMotorQ.hpp \
    Components/Hydraulic/HydraulicVolume3.hpp \
    Components/Mechanic/MechanicTorqueTransformer.hpp \
    Components/Mechanic/MechanicAngularVelocityTransformer.hpp \
    Components/Mechanic/MechanicRotationalInertia.hpp \
    Components/Mechanic/MechanicTorsionalSpring.hpp
