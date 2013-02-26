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
    code/Compgen/HydraulicLaminarOrificeCG.hpp \
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
    code/Signal/SignalComponents.h \
    code/Signal/SignalVariableTimeDelay.hpp \
    code/Hydraulic/HopsanHydraulicComponents.h \
    code/Electric/HopsanElectricComponents.h \
    components/Electric/ElectricVarResistor.hpp \
    components/Electric/ElectricUsource.hpp \
    components/Electric/ElectricUsensor.hpp \
    components/Electric/ElectricSwitch.hpp \
    components/Electric/ElectricResistor.hpp \
    components/Electric/ElectricPWMdceq.hpp \
    components/Electric/ElectricMotorGear.hpp \
    components/Electric/ElectricMotor.hpp \
    components/Electric/ElectricIsource.hpp \
    components/Electric/ElectricIsensor.hpp \
    components/Electric/ElectricInductance.hpp \
    components/Electric/ElectricIcontroller.hpp \
    components/Electric/ElectricGround.hpp \
    components/Electric/ElectricCapacitanceMultiPort.hpp \
    components/Electric/ElectricCapacitance2.hpp \
    components/Electric/ElectricBattery.hpp \
    components/Signal/Animation/SignalDisplay.hpp \
    components/Signal/Animation/SignalAnimationSlider.hpp \
    components/Signal/Animation/SignalAnimationGauge.hpp \
    components/Signal/Arithmetics/SignalAbsoluteValue.hpp \
    components/Signal/Arithmetics/SignalSum.hpp \
    components/Signal/Arithmetics/SignalSub.hpp \
    components/Signal/Arithmetics/SignalSquare.hpp \
    components/Signal/Arithmetics/SignalSin.hpp \
    components/Signal/Arithmetics/SignalSign.hpp \
    components/Signal/Arithmetics/SignalPower.hpp \
    components/Signal/Arithmetics/SignalMultiply.hpp \
    components/Signal/Arithmetics/SignalMin.hpp \
    components/Signal/Arithmetics/SignalMax.hpp \
    components/Signal/Arithmetics/SignalGain.hpp \
    components/Signal/Arithmetics/SignalDivide.hpp \
    components/Signal/Arithmetics/SignalCos.hpp \
    components/Signal/Arithmetics/SignalAdd.hpp \
    components/Signal/Arithmetics/SignalTan.hpp \
    components/Signal/Control/SignalPIlead.hpp \
    components/Signal/Control/SignalPID.hpp \
    components/Signal/Filters/SignalFirstOrderFilter.hpp \
    components/Signal/Filters/SignalFirstOrderTransferFunction.hpp \
    components/Signal/Filters/SignalHP1Filter.hpp \
    components/Signal/Filters/SignalHP2Filter.hpp \
    components/Signal/Filters/SignalLP1Filter.hpp \
    components/Signal/Filters/SignalLP2Filter.hpp \
    components/Signal/Filters/SignalSecondOrderFilter.hpp \
    components/Signal/Filters/SignalSecondOrderTransferFunction.hpp \
    components/Signal/Logic/SignalXor.hpp \
    components/Signal/Logic/SignalStopSimulation.hpp \
    components/Signal/Logic/SignalSRlatch.hpp \
    components/Signal/Logic/SignalSmallerThan.hpp \
    components/Signal/Logic/SignalOr.hpp \
    components/Signal/Logic/SignalGreaterThan.hpp \
    components/Signal/Logic/SignalAnd.hpp \
    components/Signal/Non-Linearities/SignalVariableTimeDelay.hpp \
    components/Signal/Non-Linearities/SignalUnitDelay.hpp \
    components/Signal/Non-Linearities/SignalTimeDelay.hpp \
    components/Signal/Non-Linearities/SignalLookUpTable2D.hpp \
    components/Signal/Non-Linearities/SignalHysteresis.hpp \
    components/Signal/Non-Linearities/SignalDeadZone.hpp \
    components/Signal/Non-Linearities/SignalAdditiveNoise.hpp \
    components/Signal/Signal Routing/SignalTripleRoute.hpp \
    components/Signal/Signal Routing/SignalQuadRoute.hpp \
    components/Signal/Signal Routing/SignalDualRoute.hpp \
    components/Hydraulic/HopsanDefaultHydraulicComponents.h \
    components/Hydraulic/Valves/Hydraulic22DirectionalValve.hpp \
    components/Hydraulic/Valves/HydraulicValve416.hpp \
    components/Hydraulic/Valves/HydraulicShuttleValve.hpp \
    components/Hydraulic/Valves/HydraulicPressureReliefValve.hpp \
    components/Hydraulic/Valves/HydraulicPressureReducingValve.hpp \
    components/Hydraulic/Valves/HydraulicPressureDropValve.hpp \
    components/Hydraulic/Valves/HydraulicPressureControlledValve.hpp \
    components/Hydraulic/Valves/HydraulicPressureCompensatingValve.hpp \
    components/Hydraulic/Valves/HydraulicCheckValveWithOrifice.hpp \
    components/Hydraulic/Valves/HydraulicCheckValvePreLoaded.hpp \
    components/Hydraulic/Valves/HydraulicCheckValve.hpp \
    components/Hydraulic/Valves/Hydraulic43ValveNeutralToTank.hpp \
    components/Hydraulic/Valves/Hydraulic43ValveNeutralSupplyToTank.hpp \
    components/Hydraulic/Valves/Hydraulic43Valve.hpp \
    components/Hydraulic/Valves/Hydraulic43LoadSensingValve.hpp \
    components/Hydraulic/Valves/Hydraulic42Valve.hpp \
    components/Hydraulic/Valves/Hydraulic33Valve.hpp \
    components/Hydraulic/Valves/Hydraulic33ShuttleValve.hpp \
    components/Hydraulic/Valves/Hydraulic32DirectionalValve.hpp \
    components/Hydraulic/Valves/Hydraulic22Valve.hpp \
    components/Hydraulic/Valves/Hydraulic22PoppetValve.hpp \
    components/Hydraulic/restrictors/HydraulicTurbulentOrifice.hpp \
    components/Hydraulic/restrictors/HydraulicLossLessConnector.hpp \
    components/Hydraulic/restrictors/HydraulicLaminarOrifice.hpp \
    components/Hydraulic/Restrictors/HydraulicTurbulentOrifice.hpp \
    components/Hydraulic/Restrictors/HydraulicLossLessConnector.hpp \
    components/Hydraulic/Restrictors/HydraulicLaminarOrifice.hpp \
    components/Hydraulic/Restrictors/HopsanDefaultHydraulicRestrictors.h \
    components/Hydraulic/Sensors/HydraulicPressureSensor.hpp \
    components/Hydraulic/Sensors/HydraulicFlowSensor.hpp \
    components/Hydraulic/Sensors/HopsanDefaultHydraulicSensors.h \
    components/Hydraulic/Linear Actuators/HydraulicCylinderC.hpp \
    components/Hydraulic/Linear Actuators/HydraulicCylinderQ.hpp \
    components/Hydraulic/Machine Parts/HydraulicValvePlate.hpp \
    components/Hydraulic/Pumps&Motors/HydraulicVariableDisplacementPump.hpp \
    components/Hydraulic/Pumps&Motors/HydraulicVariableDisplacementMotorQ.hpp \
    components/Hydraulic/Pumps&Motors/HydraulicPressureControlledPump.hpp \
    components/Hydraulic/Pumps&Motors/HydraulicMachineC.hpp \
    components/Hydraulic/Pumps&Motors/HydraulicFixedDisplacementPump.hpp \
    components/Hydraulic/Pumps&Motors/HydraulicFixedDisplacementMotorQ.hpp \
    components/Hydraulic/Pumps&Motors/HopsanDefaulHydraulicPumpsAndMotors.h \
    components/Hydraulic/Volumes&Lines/HydraulicVolumeMultiPort.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicVolume4.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicVolume3.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicVolume.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicTLMRLineR.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicTLMlossless.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicHose.hpp \
    components/Hydraulic/Volumes&Lines/HydraulicAckumulator.hpp \
    components/Hydraulic/Volumes&Lines/HopsanDefaulHydraulicVolumesAndLines.h \
    components/Hydraulic/Sensors/HydraulicPowerSensor.hpp \
    components/Hydraulic/Valves/HopsanDefaultHydraulicValves.h \
    components/Hydraulic/Sources&Sinks/HydraulicTankQ.hpp \
    components/Hydraulic/Sources&Sinks/HydraulicTankC.hpp \
    components/Hydraulic/Sources&Sinks/HydraulicPressureSourceQ.hpp \
    components/Hydraulic/Sources&Sinks/HydraulicPressureSourceC.hpp \
    components/Hydraulic/Sources&Sinks/HydraulicMultiTankC.hpp \
    components/Hydraulic/Sources&Sinks/HydraulicMultiPressureSourceC.hpp \
    components/Hydraulic/Sources&Sinks/HydraulicFlowSourceQ.hpp \
    components/Hydraulic/Sources&Sinks/HopsanDefaultHydraulicSourcesAndSinks.h \
    components/Hydraulic/LinearActuators/HydraulicCylinderQ.hpp \
    components/Hydraulic/LinearActuators/HydraulicCylinderC.hpp \
    components/Hydraulic/LinearActuators/HopsanDefaultHydraulicLinearActuators.h \
    components/Hydraulic/MachineParts/HydraulicValvePlate.hpp \
    components/Hydraulic/MachineParts/HopsanDefaultHydraulicMachineParts.h \
    components/Mechanic/Linear/MechanicVelocityTransformer.hpp \
    components/Mechanic/Linear/MechanicTranslationalSpring.hpp \
    components/Mechanic/Linear/MechanicTranslationalMassWithLever.hpp \
    components/Mechanic/Linear/MechanicTranslationalMassWithCoulombFriction.hpp \
    components/Mechanic/Linear/MechanicTranslationalMass.hpp \
    components/Mechanic/Linear/MechanicTranslationalLosslessConnector.hpp \
    components/Mechanic/Linear/MechanicSpeedSensor.hpp \
    components/Mechanic/Linear/MechanicPositionSensor.hpp \
    components/Mechanic/Linear/MechanicMultiPortTranslationalMass.hpp \
    components/Mechanic/Linear/MechanicFreeLengthWall.hpp \
    components/Mechanic/Linear/MechanicForceTransformer.hpp \
    components/Mechanic/Linear/MechanicForceSensor.hpp \
    components/Mechanic/Linear/MechanicFixedPosition.hpp \
    components/Mechanic/Rotational/MechanicRotationalInterfaceQ.hpp \
    components/Mechanic/Rotational/MechanicRotationalInterfaceC.hpp \
    components/Mechanic/Rotational/MechanicRotationalInertiaWithSingleGear.hpp \
    components/Mechanic/Rotational/MechanicRotationalInertiaWithGearRatio.hpp \
    components/Mechanic/Rotational/MechanicRotationalInertiaWithCoulumbFriction.hpp \
    components/Mechanic/Rotational/MechanicRotationalInertia.hpp \
    components/Mechanic/Linear/HopsanDefaultMechanicLinearComponents.h \
    components/Mechanic/Rotational/HopsanDefaultMechanicRotationalComponents.h \
    components/Connectivity/SignalOutputInterface.hpp \
    components/Connectivity/SignalInputInterface.hpp \
    components/Connectivity/MechanicInterfaceQ.hpp \
    components/Connectivity/MechanicInterfaceC.hpp \
    components/Connectivity/HydraulicInterfaceQ.hpp \
    components/Connectivity/HydraulicInterfaceC.hpp \
    components/Connectivity/HopsanDefaultconnectivityComponents.h \
    components/Signal/Arithmetics/HopsanDefaultSignalArithmeticComponents.h \
    components/Signal/Sources&Sinks/SignalTime.hpp \
    components/Signal/Sources&Sinks/SignalStepExponentialDelay.hpp \
    components/Signal/Sources&Sinks/SignalStep.hpp \
    components/Signal/Sources&Sinks/SignalStaircase.hpp \
    components/Signal/Sources&Sinks/SignalSquareWave.hpp \
    components/Signal/Sources&Sinks/SignalSource.hpp \
    components/Signal/Sources&Sinks/SignalSoftStep.hpp \
    components/Signal/Sources&Sinks/SignalSink.hpp \
    components/Signal/Sources&Sinks/SignalSineWave.hpp \
    components/Signal/Sources&Sinks/SignalRamp.hpp \
    components/Signal/Sources&Sinks/SignalPulseWave.hpp \
    components/Signal/Sources&Sinks/SignalPulse.hpp \
    components/Signal/Sources&Sinks/SignalNoiseGenerator.hpp \
    components/Signal/SignalRouting/SignalDualRoute.hpp \
    components/Signal/SignalRouting/SignalQuadRoute.hpp \
    components/Signal/SignalRouting/SignalTripleRoute.hpp \
    components/Signal/Sources&Sinks/HopsanDefaultSignalSourcesAndSinks.h \
    components/Signal/SignalRouting/HopsanDefaultSignalRoutingComponents.h \
    components/Signal/Non-Linearities/HopsanDefaultSignalNonLinearities.h \
    components/Signal/Control/HopsanDefaultSignalControlComponents.h \
    components/Signal/Logic/HopsanDefaultSignalLogicComponents.h \
    components/Signal/Animation/HopsanDefaultSignalAnimationComponents.h \
    components/Signal/Filters/HopsanDefaultSignalTFComponents.h \
    components/Signal/Arithmetics/SignalSubtract.hpp \
    components/Special/Benchmarking/SignalDummy.hpp \
    components/Special/Benchmarking/HydraulicDummyQ.hpp \
    components/Special/Benchmarking/HydraulicDummyC.hpp \
    components/Special/SignalFFB/SignalFFBorIn.hpp \
    components/Special/SignalFFB/SignalFFBor.hpp \
    components/Special/SignalFFB/SignalFFBloopIn.hpp \
    components/Special/SignalFFB/SignalFFBloop.hpp \
    components/Special/SignalFFB/SignalFFBandIn.hpp \
    components/Special/SignalFFB/SignalFFBand.hpp \
    components/Special/SignalFFB/SignalFFB.hpp \
    components/Special/Aerocomponents/SignalWaypoint.hpp \
    components/Special/Aerocomponents/SignalTimeAccelerator.hpp \
    components/Special/Aerocomponents/SignalStateMonitor.hpp \
    components/Special/Aerocomponents/SignalEarthCoordinates.hpp \
    components/Special/Aerocomponents/SignalAttitudeTVCcontrol.hpp \
    components/Special/Aerocomponents/SignalAttitudeControl.hpp \
    components/Special/AeroComponents/AeroVehicleTVC.hpp \
    components/Special/AeroComponents/AeroPropeller.hpp \
    components/Special/AeroComponents/AeroJetEngine.hpp \
    components/Special/AeroComponents/AeroFuelTank.hpp \
    components/Special/AeroComponents/AeroAtmosphere.hpp \
    components/Special/AeroComponents/AeroAircraft6DOFS.hpp \
    components/Special/AeroComponents/AeroAircraft6DOF.hpp \
    components/Special/AeroComponents/HopsanDefaultAerocomponents.h \
    components/Special/Benchmarking/HopsanDefaultBenchmarkingComponents.h \
    components/Special/SignalFFB/HopsanDefaultFBBComponents.h \
    components/Special/HopsanDefaultSpecialComponents.h \
    components/Special/MechanicVehicle1D.hpp \
    components/Hydraulic/MachineParts/MechanicSwashPlate.hpp \
    components/Hydraulic/MachineParts/MechanicCylinderBlockWithSwashPlate.hpp \
    components/Special/AeroComponents/Fuelcomponents/HydraulicFuelTankG.hpp \
    components/Special/AeroComponents/Fuelcomponents/HydraulicCentrifugalPumpJ.hpp \
    components/Special/AeroComponents/Fuelcomponents/HydraulicCentrifugalPump.hpp \
    components/Hydraulic/Valves/HydraulicOverCenterValve.hpp \
    components/Hydraulic/Valves/HydraulicOpenCenterValve.hpp \
    components/Hydraulic/Restrictors/HydraulicLossLessTConnector.hpp \
    components/Components.h \
    components/defaultComponentLibraryInternal.h \
    components/Signal/HopsanDefaultSignalComponents.h \
    components/Mechanic/HopsanDefaultMechanicComponents.h \
    components/Electric/HopsanDefaultElectricComponents.h

SOURCES += \
    components/defaultComponentLibraryInternal.cc \
    components/defaultComponentLibrary.cc
