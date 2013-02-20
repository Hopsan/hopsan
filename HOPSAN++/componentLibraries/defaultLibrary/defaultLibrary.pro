# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# TARGET is the name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = defaultComponentLibrary

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/components

# The location to search for the Hopsan include files, by specifying the path here, you dont need to do this everywhere in all of your component .hpp files
# You can also add additional paths for eg. your own Utility functions, just add additional INCLUDEPATH *= ... lines.
# *= Means append unique
INCLUDEPATH *= $${PWD}/code/
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/

# The location of the HopsanCore .dll or .so file, needed to link against when compiling your library
LIBS *= -L$${PWD}/../../bin

# Special options for deug and release mode
# In debug mode HopsanCore has the debug extension _d
CONFIG(debug, debug|release) {
    LIBS *= -lHopsanCore_d
    DEFINES *= DEBUGCOMPILING
}
CONFIG(release, debug|release) {
    LIBS *= -lHopsanCore
    DEFINES *= RELEASECOMPILING
}

# Reduce compile output clutter, but show warnings
CONFIG += silent warn_on

# The compiler should be pedantic to catch all errors (optional)
QMAKE_CXXFLAGS += -pedantic

# -------------------------------------------------
# Project files
# -------------------------------------------------
HEADERS += \
    code/Components.h \
    code/Special/AeroComponents/AeroAtmosphere.hpp \
    code/Special/AeroComponents/AeroJetEngine.hpp \
    code/Special/AeroComponents/AeroFuelTank.hpp \
    code/Special/AeroComponents/AeroPropeller.hpp \
    code/Special/AeroComponents/AeroAircraft6DOF.hpp \
    code/Special/AeroComponents/AeroAircraft6DOFS.hpp \
    code/Special/SignalFFB/SignalFFB.hpp \
    code/Special/SignalFFB/SignalFFBand.hpp \
    code/Special/SignalFFB/SignalFFBandIn.hpp \
    code/Special/SignalFFB/SignalFFBor.hpp \
    code/Special/SignalFFB/SignalFFBorIn.hpp \
    code/Special/SignalFFB/SignalFFBloop.hpp \
    code/Special/SignalFFB/SignalFFBloopIn.hpp \
    code/Special/AeroComponents/SignalAttitudeControl.hpp \
    code/Special/AeroComponents/SignalAttitudeTVCcontrol.hpp \
    code/Special/AeroComponents/SignalWaypointController.hpp \
    code/Special/AeroComponents/SignalWaypoint.hpp \
    code/Special/AeroComponents/SignalStateMonitor.hpp \
    code/Special/AeroComponents/SignalTimeAccelerator.hpp \
    code/Special/AeroComponents/SignalEarthCoordinates.hpp \
    code/Signal/SignalSRlatch.hpp \
    code/Signal/SignalXor.hpp \
    code/Signal/SignalUndefinedConnection.hpp \
    code/Signal/SignalTripleRoute.hpp \
    code/Signal/SignalTimeDelay.hpp \
    code/Signal/SignalTime.hpp \
    code/Signal/SignalTan.hpp \
    code/Signal/SignalSum.hpp \
    code/Signal/SignalSubtract.hpp \
    code/Signal/SignalStopSimulation.hpp \
    code/Signal/SignalStepExponentialDelay.hpp \
    code/Signal/SignalStep.hpp \
    code/Signal/SignalSquareWave.hpp \
    code/Signal/SignalSquare.hpp \
    code/Signal/SignalSource.hpp \
    code/Signal/SignalSoftStep.hpp \
    code/Signal/SignalSmallerThan.hpp \
    code/Signal/SignalSink.hpp \
    code/Signal/SignalSineWave.hpp \
    code/Signal/SignalSin.hpp \
    code/Signal/SignalSecondOrderTransferFunction.hpp \
    code/Signal/SignalSecondOrderFilter.hpp \
    code/Signal/SignalSaturation.hpp \
    code/Signal/SignalRamp.hpp \
    code/Signal/SignalQuadRoute.hpp \
    code/Signal/SignalPulse.hpp \
    code/Signal/SignalPower.hpp \
    code/Signal/SignalPIlead.hpp \
    code/Signal/SignalPID.hpp \
    code/Signal/SignalOutputInterface.hpp \
    code/Signal/SignalOr.hpp \
    code/Signal/SignalNoiseGenerator.hpp \
    code/Signal/SignalMultiply.hpp \
    code/Signal/SignalMin.hpp \
    code/Signal/SignalMax.hpp \
    code/Signal/SignalLP2Filter.hpp \
    code/Signal/SignalLP1Filter.hpp \
    code/Signal/SignalLookUpTable2D.hpp \
    code/Signal/SignalIntegratorLimited2.hpp \
    code/Signal/SignalIntegratorLimited.hpp \
    code/Signal/SignalIntegrator2.hpp \
    code/Signal/SignalIntegrator.hpp \
    code/Signal/SignalInputInterface.hpp \
    code/Signal/SignalHysteresis.hpp \
    code/Signal/SignalHP2Filter.hpp \
    code/Signal/SignalHP1Filter.hpp \
    code/Signal/SignalGreaterThan.hpp \
    code/Signal/SignalGain.hpp \
    code/Signal/SignalFirstOrderTransferFunction.hpp \
    code/Signal/SignalFirstOrderFilter.hpp \
    code/Signal/SignalDummy.hpp \
    code/Signal/SignalDualRoute.hpp \
    code/Signal/SignalDivide.hpp \
    code/Signal/SignalDeadZone.hpp \
    code/Signal/SignalCos.hpp \
    code/Signal/SignalBETest.hpp \
    code/Signal/SignalAttitude.hpp \
    code/Signal/SignalAnd.hpp \
    code/Signal/SignalAdditiveNoise.hpp \
    code/Signal/SignalAdd.hpp \
    code/Signal/SignalAbsoluteValue.hpp \
    code/Mechanic/MechanicVelocityTransformer.hpp \
    code/Mechanic/MechanicVehicle1D.hpp \
    code/Mechanic/MechanicTranslationalSpring.hpp \
    code/Mechanic/MechanicTranslationalMassWithLever.hpp \
    code/Mechanic/MechanicTranslationalMassWithCoulombFriction.hpp \
    code/Mechanic/MechanicTranslationalMass.hpp \
    code/Mechanic/MechanicTranslationalLosslessConnector.hpp \
    code/Mechanic/MechanicTorsionalSpring.hpp \
    code/Mechanic/MechanicTorqueTransformer.hpp \
    code/Mechanic/MechanicTorqueSensor.hpp \
    code/Mechanic/MechanicThetaSource.hpp \
    code/Mechanic/MechanicSpeedSensor.hpp \
    code/Mechanic/MechanicRotationalInterfaceQ.hpp \
    code/Mechanic/MechanicRotationalInterfaceC.hpp \
    code/Mechanic/MechanicRotationalInertiaWithSingleGear.hpp \
    code/Mechanic/MechanicRotationalInertiaWithGearRatio.hpp \
    code/Mechanic/MechanicRotationalInertiaWithCoulumbFriction.hpp \
    code/Mechanic/MechanicRotationalInertia.hpp \
    code/Mechanic/MechanicRotShaft.hpp \
    code/Mechanic/MechanicRackAndPinion.hpp \
    code/Mechanic/MechanicPositionSensor.hpp \
    code/Mechanic/MechanicMultiPortTranslationalMass.hpp \
    code/Mechanic/MechanicJLink.hpp \
    code/Mechanic/MechanicInterfaceQ.hpp \
    code/Mechanic/MechanicInterfaceC.hpp \
    code/Mechanic/MechanicFreeLengthWall.hpp \
    code/Mechanic/MechanicForceTransformer.hpp \
    code/Mechanic/MechanicForceSensor.hpp \
    code/Mechanic/MechanicFixedPosition.hpp \
    code/Mechanic/MechanicAngularVelocityTransformer.hpp \
    code/Mechanic/MechanicAngularVelocitySensor.hpp \
    code/Mechanic/MechanicAngleSensor.hpp \
    code/Hydraulic/HydraulicVolumeMultiPort.hpp \
    code/Hydraulic/HydraulicVolume4.hpp \
    code/Hydraulic/HydraulicVolume3.hpp \
    code/Hydraulic/HydraulicVolume.hpp \
    code/Hydraulic/HydraulicVariableDisplacementPump.hpp \
    code/Hydraulic/HydraulicVariableDisplacementMotorQ.hpp \
    code/Hydraulic/HydraulicUndefinedConnectionQ.hpp \
    code/Hydraulic/HydraulicUndefinedConnectionC.hpp \
    code/Hydraulic/HydraulicTurbulentOrifice.hpp \
    code/Hydraulic/HydraulicTLMRLineR.hpp \
    code/Hydraulic/HydraulicTLMlossless.hpp \
    code/Hydraulic/HydraulicTankQ.hpp \
    code/Hydraulic/HydraulicTankC.hpp \
    code/Hydraulic/HydraulicSubSysExample.hpp \
    code/Hydraulic/HydraulicPressureSourceQ.hpp \
    code/Hydraulic/HydraulicPressureSourceC.hpp \
    code/Hydraulic/HydraulicPressureSensor.hpp \
    code/Hydraulic/HydraulicPressureControlledPump.hpp \
    code/Hydraulic/HydraulicPowerSensor.hpp \
    code/Hydraulic/HydraulicPilotControlledCheckValve.hpp \
    code/Hydraulic/HydraulicPilotClosableCheckValve.hpp \
    code/Hydraulic/HydraulicOverCenterValve.hpp \
    code/Hydraulic/HydraulicOpenCenterValve.hpp \
    code/Hydraulic/HydraulicMultiTankC.hpp \
    code/Hydraulic/HydraulicMultiPressureSourceC.hpp \
    code/Hydraulic/HydraulicMachineC.hpp \
    code/Hydraulic/HydraulicLossLessTConnector.hpp \
    code/Hydraulic/HydraulicLossLessConnector.hpp \
    code/Hydraulic/HydraulicLaminarOrifice.hpp \
    code/Hydraulic/HydraulicInterfaceQ.hpp \
    code/Hydraulic/HydraulicInterfaceC.hpp \
    code/Hydraulic/HydraulicHose.hpp \
    code/Hydraulic/HydraulicFlowSourceQ.hpp \
    code/Hydraulic/HydraulicFlowSensor.hpp \
    code/Hydraulic/HydraulicFixedDisplacementPump.hpp \
    code/Hydraulic/HydraulicFixedDisplacementMotorQ.hpp \
    code/Hydraulic/HydraulicDummyQ.hpp \
    code/Hydraulic/HydraulicDummyC.hpp \
    code/Hydraulic/HydraulicCylinderQ.hpp \
    code/Hydraulic/HydraulicCylinderC.hpp \
    code/Hydraulic/HydraulicAlternativePRV.hpp \
    code/Hydraulic/HydraulicAckumulator.hpp \
    code/Hydraulic/HydraulicCentrifugalPump.hpp \
    code/Hydraulic/HydraulicCentrifugalPumpJ.hpp \
    code/Hydraulic/HydraulicFuelTankG.hpp \
    code/Hydraulic/valves/HydraulicValves.h \
    code/Hydraulic/valves/HydraulicValve416.hpp \
    code/Hydraulic/valves/HydraulicShuttleValve.hpp \
    code/Hydraulic/valves/HydraulicPressureReliefValve.hpp \
    code/Hydraulic/valves/HydraulicPressureReducingValve.hpp \
    code/Hydraulic/valves/HydraulicPressureDropValve.hpp \
    code/Hydraulic/valves/HydraulicPressureControlledValve.hpp \
    code/Hydraulic/valves/HydraulicPressureCompensatingValve.hpp \
    code/Hydraulic/valves/HydraulicCheckValveWithOrifice.hpp \
    code/Hydraulic/valves/HydraulicCheckValve.hpp \
    code/Hydraulic/valves/Hydraulic43ValveNeutralToTank.hpp \
    code/Hydraulic/valves/Hydraulic43ValveNeutralSupplyToTank.hpp \
    code/Hydraulic/valves/Hydraulic43Valve.hpp \
    code/Hydraulic/valves/Hydraulic43LoadSensingValve.hpp \
    code/Hydraulic/valves/Hydraulic42Valve.hpp \
    code/Hydraulic/valves/Hydraulic33Valve.hpp \
    code/Hydraulic/valves/Hydraulic33ShuttleValve.hpp \
    code/Hydraulic/valves/Hydraulic32DirectionalValve.hpp \
    code/Hydraulic/valves/Hydraulic22Valve.hpp \
    code/Hydraulic/valves/Hydraulic22PoppetValve.hpp \
    code/Hydraulic/valves/Hydraulic22DirectionalValve.hpp \
    code/Electric/ElectricVarResistor.hpp \
    code/Electric/ElectricUsource.hpp \
    code/Electric/ElectricUsensor.hpp \
    code/Electric/ElectricSwitch.hpp \
    code/Electric/ElectricResistor.hpp \
    code/Electric/ElectricMotorGear.hpp \
    code/Electric/ElectricMotor.hpp \
    code/Electric/ElectricIsource.hpp \
    code/Electric/ElectricIsensor.hpp \
    code/Electric/ElectricInductance.hpp \
    code/Electric/ElectricIcontroller.hpp \
    code/Electric/ElectricPWMdceq.hpp \
    code/Electric/ElectricGround.hpp \
    code/Electric/ElectricCapacitance2.hpp \
    code/Electric/ElectricCapacitanceMultiPort.hpp \
    code/Electric/ElectricBattery.hpp \
    code/Electric/ElectricPWMdceq.hpp \
    code/Compgen/HydraulicLaminarOrificeCG.hpp \
    code/defaultComponentLibraryInternal.h \
    code/Signal/SignalSign.hpp \
    code/Signal/SignalSub.hpp \
    code/Hydraulic/valves/HydraulicCheckValvePreLoaded.hpp \
    code/Signal/SignalPulseWave.hpp \
    code/Signal/SignalAnimationSlider.hpp \
    code/Hydraulic/HydraulicValvePlate.hpp \
    code/Hydraulic/HydraulicPumpPiston.hpp \
    code/Mechanic/MechanicSwashPlate.hpp \
    code/Mechanic/MechanicMotor.hpp \
    code/Mechanic/MechanicCylinderBlockWithSwashPlate.hpp \
    code/Signal/SignalStaircase.hpp \
    code/Signal/SignalDisplay.hpp \
    code/Signal/SignalAnimationGauge.hpp \
    code/Signal/SignalUnitDelay.hpp \
    code/Special/AeroComponents/AeroVehicleTVC.hpp

SOURCES += \
    code/defaultComponentLibrary.cc \
    code/defaultComponentLibraryInternal.cc
