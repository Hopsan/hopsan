# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( HopsanCoreBuild.prf )

TARGET = HopsanCore
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT -= core \
    gui

TARGET = $${TARGET}$${DEBUG_EXT}

#--------------------------------------------------------
# Set the rappidxml include path
INCLUDEPATH *= $${PWD}/../ExternalDependencies/rapidxml-1.13
INCLUDEPATH *= $${PWD}/../ExternalDependencies/libcsv_parser++-1.0.0/include/csv_parser
#--------------------------------------------------------

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the core into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the core as a DLL or SO
    DEFINES -= UNICODE

    #--------------------------------------------------------
    # Set the TBB LIBS and INCLUDEPATH (helpfunction for Windows)
    d = $$setTBBWindowsPathInfo($$(TBB_PATH), $$DESTDIR)
    !isEmpty(d){
        #DEFINES *= USETBB       #If TBB was found then lets build core with TBB support
        #message(Compiling HopsanCore with TBB support)
        #LIBS *= $$magic_hopsan_libpath
        #INCLUDEPATH *= $$magic_hopsan_includepath
        #QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
    }
    #--------------------------------------------------------
}
unix { 
    LIBS += -ltbb
    INCLUDEPATH += /usr/include/tbb/
}

#Debug output
#message(CORE QMAKE_POST_LINK $${QMAKE_POST_LINK})
#message(CORE Includepath is $$INCLUDEPATH)
#message(CORE Libs is $${LIBS})
#message(CORE Defines is $${DEFINES})

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += Port.cc \
    Node.cc \
    Component.cc \
    Nodes/Nodes.cc \
    CoreUtilities/LoadExternal.cc \
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
    ComponentUtilities/matrix.cc \
    ComponentUtilities/ludcmp.cc \
    CoreUtilities/HmfLoader.cc \
    ComponentSystem.cc \
    ../ExternalDependencies/libcsv_parser++-1.0.0/csv_parser.cpp
HEADERS += win32dll.h \
    Port.h \
    Node.h \
    HopsanCore.h \
    Component.h \
    Nodes/Nodes.h \
    CoreUtilities/LoadExternal.h \
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
    Components/Hydraulic/HydraulicPressureCompensatingValve.hpp \
    Components/Hydraulic/HydraulicUndefinedConnectionQ.hpp \
    Components/Hydraulic/HydraulicUndefinedConnectionC.hpp \
    Components/Signal/SignalMax.hpp \
    Components/Signal/SignalMin.hpp \
    Components/Signal/SignalRoute.hpp \
    Components/Signal/SignalSecondOrderTransferFunction.hpp \
    Components/Hydraulic/HydraulicLossLessTConnector.hpp \
    Components/Hydraulic/HydraulicLossLessConnector.hpp \
    CoreUtilities/HmfLoader.h \
    ComponentSystem.h \
    CoreUtilities/FindUniqueName.h \
    Components/Mechanic/MechanicMultiPortTranslationalMass.hpp \
    Components/Hydraulic/Hydraulic43ValveNeutralToTank.hpp \
    Components/Hydraulic/Hydraulic43ValveNeutralSupplyToTank.hpp \
    Components/Hydraulic/HydraulicHose.hpp \
    ../ExternalDependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp \
    ComponentUtilities/ReadDataCurve.h \
    ComponentUtilities/CSVParser.h \
    ComponentUtilities/ReadDataCurve.h \
    ComponentUtilities/CSVParser.h \
    Components/Signal/SignalPower.hpp \
    Components/Signal/SignalUndefinedConnection.hpp \
    Components/Hydraulic/HydraulicPilotControlledCheckValve.hpp \
    Components/Hydraulic/HydraulicCheckValveWithOrifice.hpp \
    Components/Hydraulic/HydraulicPilotClosableCheckValve.hpp \
    Components/Signal/SignalOutputInterface.hpp \
    Components/Signal/SignalInputInterface.hpp \
    Components/Mechanic/MechanicRotationalInterfaceQ.hpp \
    Components/Mechanic/MechanicRotationalInterfaceC.hpp \
    Components/Mechanic/MechanicInterfaceQ.hpp \
    Components/Mechanic/MechanicInterfaceC.hpp

OTHER_FILES += \
    HopsanCoreBuild.prf
