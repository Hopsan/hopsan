# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = HopsanCore
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../lib

QT -= core \
    gui

TARGET = $${TARGET}$${DEBUG_EXT}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the core into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the core as a DLL or SO
    DEFINES -= UNICODE

    #Set default tbb path alternatives, higher up is prefered
    TBB_PATHS *= $${PWD}/../ExternalDependencies/tbb30_20101215oss
    TBB_PATHS *= $${PWD}/../ExternalDependencies/tbb30_20100406oss
    #Try environment variable first $$(ENVVARNAME)if it exists, then default paths listed above
    TBB_PATH = $$selectPath($$(TBB_PATH), $$TBB_PATHS, "tbb")
    exists($${TBB_PATH}) {
        INCLUDEPATH += $${TBB_PATH}/include/tbb

        CONFIG(debug, debug|release) {
            LIBS += -L$${TBB_PATH}/build/windows_ia32_gcc_mingw_debug
            LIBS += -ltbb_debug
        }
        CONFIG(release, debug|release) {
            LIBS += -L$${TBB_PATH}/build/windows_ia32_gcc_mingw_release
            LIBS += -ltbb
        }
        DEFINES += USETBB       #If TBB was found then let build core with TBB support
    }

    #Debug output
    #message(Includepath is $$INCLUDEPATH)
    #message(Libs is $${LIBS})
    #message(Defines is $${DEFINES})
}
unix { 
    LIBS += -ltbb
    INCLUDEPATH += /usr/include/tbb/
}

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
    ComponentUtilities/SecondOrderFilter.cc \
    ComponentUtilities/IntegratorLimited.cc \
    ComponentUtilities/Integrator.cc \
    ComponentUtilities/FirstOrderFilter.cc \
    ComponentUtilities/Delay.cc \
    CoreUtilities/HopsanCoreMessageHandler.cc \
    ComponentUtilities/AuxiliarySimulationFunctions.cpp \
    ComponentUtilities/DoubleIntegratorWithDamping.cpp \
    ComponentUtilities/SecondOrderTransferFunction.cc \
    ComponentUtilities/matrix.cc \
    ComponentUtilities/ludcmp.cc
HEADERS += win32dll.h \
    Port.h \
    Node.h \
    HopsanCore.h \
    Component.h \
    Nodes/Nodes.h \
    CoreUtilities/LoadExternal.h \
    CoreUtilities/FileAccess.h \
    CoreUtilities/ClassFactory.hpp \
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
    Components/Mechanic/MechanicTorsionalSpring.hpp \
    Components/Mechanic/MechanicAngleSensor.hpp \
    Components/Signal/SignalStopSimulation.hpp \
    Components/Signal/SignalGreaterThan.hpp \
    Components/Signal/SignalSmallerThan.hpp \
    Components/Signal/SignalAnd.hpp \
    Components/Signal/SignalOr.hpp \
    Components/Signal/SignalXor.hpp \
    Components/Signal/SignalTime.hpp \
    ComponentUtilities/SecondOrderTransferFunction.h \
    Components/Signal/SignalDummy.hpp \
    Components/Hydraulic/HydraulicDummyC.hpp \
    Components/Hydraulic/HydraulicDummyQ.hpp \
    Components/Hydraulic/Hydraulic33Valve.hpp \
    Components/Signal/SignalHP1Filter.hpp \
    Components/Signal/SignalHP2Filter.hpp \
    Components/Hydraulic/Hydraulic43LoadSensingValve.hpp \
    Components/Hydraulic/Hydraulic22Valve.hpp \
    Components/Hydraulic/Hydraulic42Valve.hpp \
    Components/Hydraulic/HydraulicOpenCenterValve.hpp \
    Components/Hydraulic/HydraulicMachineC.hpp \
    Components/Hydraulic/HydraulicVolume4.hpp \
    Components/Mechanic/MechanicTranslationalMassWithCoulumbFriction.hpp \
    Components/Hydraulic/Hydraulic33Valve.hpp \
    Components/Hydraulic/Hydraulic33Valve.hpp \
    Components/Hydraulic/Hydraulic32DirectionalValve.hpp \
    Components/HydraulicComponentTemplate.hpp \
    Components/Mechanic/MechanicTranslationalLosslessConnector.hpp \
    Components/Hydraulic/HydraulicPressureControlledValve.hpp \
    ComponentUtilities/matrix.h \
    ComponentUtilities/ludcmp.h \
    Components/Hydraulic/HydraulicShuttleValve.hpp \
    Components/Hydraulic/HydraulicPressureControlledPump.hpp \
    Components/Hydraulic/HydraulicPressureDropValve.hpp \
    Components/Hydraulic/HydraulicPressureReducingValve.hpp \
    Components/Signal/SignalAbsoluteValue.hpp \
    Components/Hydraulic/HydraulicTankC.hpp \
    Components/Hydraulic/HydraulicTankQ.hpp \
    Components/Hydraulic/HydraulicPressureSourceC.hpp \
    Components/Mechanic/MechanicTorqueSensor.hpp \
    Components/Hydraulic/Hydraulic22DirectionalValve.hpp \
    Components/Mechanic/MechanicTranslationalMassWithLever.hpp \
    Components/Mechanic/MechanicRotationalInertiaWithGearRatio.hpp \
    Components/Mechanic/MechanicRotationalInertiaWithSingleGear.hpp \
    Components/Hydraulic/HydraulicMultiPressureSourceC.hpp \
    Components/Hydraulic/HydraulicVolumeMultiPort.hpp \
    Components/Signal/SignalSum.hpp \
    Components/Hydraulic/HydraulicPressureCompensatingValve.hpp
