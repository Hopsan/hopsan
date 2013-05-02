# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# TARGET is the name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = defaultComponentLibrary

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

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
QMAKE_CXXFLAGS += -pedantic -Wno-long-long

# -------------------------------------------------
# Project files
# -------------------------------------------------
HEADERS += \
    Electric/ElectricVarResistor.hpp \
    Electric/ElectricUsource.hpp \
    Electric/ElectricUsensor.hpp \
    Electric/ElectricSwitch.hpp \
    Electric/ElectricResistor.hpp \
    Electric/ElectricPWMdceq.hpp \
    Electric/ElectricMotorGear.hpp \
    Electric/ElectricMotor.hpp \
    Electric/ElectricIsource.hpp \
    Electric/ElectricIsensor.hpp \
    Electric/ElectricInductance.hpp \
    Electric/ElectricIcontroller.hpp \
    Electric/ElectricGround.hpp \
    Electric/ElectricCapacitanceMultiPort.hpp \
    Electric/ElectricCapacitance2.hpp \
    Electric/ElectricBattery.hpp \
    Signal/Animation/SignalDisplay.hpp \
    Signal/Animation/SignalAnimationSlider.hpp \
    Signal/Animation/SignalAnimationGauge.hpp \
    Signal/Arithmetics/SignalAbsoluteValue.hpp \
    Signal/Arithmetics/SignalSum.hpp \
    Signal/Arithmetics/SignalSub.hpp \
    Signal/Arithmetics/SignalSquare.hpp \
    Signal/Arithmetics/SignalSin.hpp \
    Signal/Arithmetics/SignalSign.hpp \
    Signal/Arithmetics/SignalPower.hpp \
    Signal/Arithmetics/SignalMultiply.hpp \
    Signal/Arithmetics/SignalMin.hpp \
    Signal/Arithmetics/SignalMax.hpp \
    Signal/Arithmetics/SignalGain.hpp \
    Signal/Arithmetics/SignalDivide.hpp \
    Signal/Arithmetics/SignalCos.hpp \
    Signal/Arithmetics/SignalAdd.hpp \
    Signal/Arithmetics/SignalTan.hpp \
    Signal/Control/SignalPIlead.hpp \
    Signal/Control/SignalPID.hpp \
    Signal/Filters/SignalFirstOrderFilter.hpp \
    Signal/Filters/SignalFirstOrderTransferFunction.hpp \
    Signal/Filters/SignalHP1Filter.hpp \
    Signal/Filters/SignalHP2Filter.hpp \
    Signal/Filters/SignalLP1Filter.hpp \
    Signal/Filters/SignalLP2Filter.hpp \
    Signal/Filters/SignalSecondOrderFilter.hpp \
    Signal/Filters/SignalSecondOrderTransferFunction.hpp \
    Signal/Logic/SignalXor.hpp \
    Signal/Logic/SignalStopSimulation.hpp \
    Signal/Logic/SignalSRlatch.hpp \
    Signal/Logic/SignalSmallerThan.hpp \
    Signal/Logic/SignalOr.hpp \
    Signal/Logic/SignalGreaterThan.hpp \
    Signal/Logic/SignalAnd.hpp \
    Signal/Non-Linearities/SignalVariableTimeDelay.hpp \
    Signal/Non-Linearities/SignalUnitDelay.hpp \
    Signal/Non-Linearities/SignalTimeDelay.hpp \
    Signal/Non-Linearities/SignalLookUpTable2D.hpp \
    Signal/Non-Linearities/SignalHysteresis.hpp \
    Signal/Non-Linearities/SignalDeadZone.hpp \
    Signal/Non-Linearities/SignalAdditiveNoise.hpp \
    Hydraulic/HopsanDefaultHydraulicComponents.h \
    Hydraulic/Valves/Hydraulic22DirectionalValve.hpp \
    Hydraulic/Valves/HydraulicValve416.hpp \
    Hydraulic/Valves/HydraulicShuttleValve.hpp \
    Hydraulic/Valves/HydraulicPressureReliefValve.hpp \
    Hydraulic/Valves/HydraulicPressureReducingValve.hpp \
    Hydraulic/Valves/HydraulicPressureDropValve.hpp \
    Hydraulic/Valves/HydraulicPressureControlledValve.hpp \
    Hydraulic/Valves/HydraulicPressureCompensatingValve.hpp \
    Hydraulic/Valves/HydraulicCheckValveWithOrifice.hpp \
    Hydraulic/Valves/HydraulicCheckValvePreLoaded.hpp \
    Hydraulic/Valves/HydraulicCheckValve.hpp \
    Hydraulic/Valves/Hydraulic43ValveNeutralToTank.hpp \
    Hydraulic/Valves/Hydraulic43ValveNeutralSupplyToTank.hpp \
    Hydraulic/Valves/Hydraulic43Valve.hpp \
    Hydraulic/Valves/Hydraulic43LoadSensingValve.hpp \
    Hydraulic/Valves/Hydraulic42Valve.hpp \
    Hydraulic/Valves/Hydraulic33Valve.hpp \
    Hydraulic/Valves/Hydraulic33ShuttleValve.hpp \
    Hydraulic/Valves/Hydraulic32DirectionalValve.hpp \
    Hydraulic/Valves/Hydraulic22Valve.hpp \
    Hydraulic/Valves/Hydraulic22PoppetValve.hpp \
    Hydraulic/Restrictors/HydraulicTurbulentOrifice.hpp \
    Hydraulic/Restrictors/HydraulicLossLessConnector.hpp \
    Hydraulic/Restrictors/HydraulicLaminarOrifice.hpp \
    Hydraulic/Restrictors/HopsanDefaultHydraulicRestrictors.h \
    Hydraulic/Sensors/HydraulicPressureSensor.hpp \
    Hydraulic/Sensors/HydraulicFlowSensor.hpp \
    Hydraulic/Sensors/HopsanDefaultHydraulicSensors.h \
    Hydraulic/Pumps&Motors/HydraulicVariableDisplacementPump.hpp \
    Hydraulic/Pumps&Motors/HydraulicVariableDisplacementMotorQ.hpp \
    Hydraulic/Pumps&Motors/HydraulicPressureControlledPump.hpp \
    Hydraulic/Pumps&Motors/HydraulicMachineC.hpp \
    Hydraulic/Pumps&Motors/HydraulicFixedDisplacementPump.hpp \
    Hydraulic/Pumps&Motors/HydraulicFixedDisplacementMotorQ.hpp \
    Hydraulic/Pumps&Motors/HopsanDefaulHydraulicPumpsAndMotors.h \
    Hydraulic/Volumes&Lines/HydraulicVolumeMultiPort.hpp \
    Hydraulic/Volumes&Lines/HydraulicVolume.hpp \
    Hydraulic/Volumes&Lines/HydraulicTLMRLineR.hpp \
    Hydraulic/Volumes&Lines/HydraulicTLMlossless.hpp \
    Hydraulic/Volumes&Lines/HydraulicHose.hpp \
    Hydraulic/Volumes&Lines/HydraulicAckumulator.hpp \
    Hydraulic/Volumes&Lines/HopsanDefaulHydraulicVolumesAndLines.h \
    Hydraulic/Sensors/HydraulicPowerSensor.hpp \
    Hydraulic/Valves/HopsanDefaultHydraulicValves.h \
    Hydraulic/Sources&Sinks/HydraulicTankC.hpp \
    Hydraulic/Sources&Sinks/HydraulicPressureSourceQ.hpp \
    Hydraulic/Sources&Sinks/HydraulicPressureSourceC.hpp \
    Hydraulic/Sources&Sinks/HydraulicMultiTankC.hpp \
    Hydraulic/Sources&Sinks/HydraulicMultiPressureSourceC.hpp \
    Hydraulic/Sources&Sinks/HydraulicFlowSourceQ.hpp \
    Hydraulic/Sources&Sinks/HopsanDefaultHydraulicSourcesAndSinks.h \
    Hydraulic/LinearActuators/HydraulicCylinderQ.hpp \
    Hydraulic/LinearActuators/HydraulicCylinderC.hpp \
    Hydraulic/LinearActuators/HopsanDefaultHydraulicLinearActuators.h \
    Hydraulic/MachineParts/HydraulicValvePlate.hpp \
    Hydraulic/MachineParts/HopsanDefaultHydraulicMachineParts.h \
    Mechanic/Linear/MechanicVelocityTransformer.hpp \
    Mechanic/Linear/MechanicTranslationalSpring.hpp \
    Mechanic/Linear/MechanicTranslationalMassWithLever.hpp \
    Mechanic/Linear/MechanicTranslationalMassWithCoulombFriction.hpp \
    Mechanic/Linear/MechanicTranslationalMass.hpp \
    Mechanic/Linear/MechanicTranslationalLosslessConnector.hpp \
    Mechanic/Linear/MechanicSpeedSensor.hpp \
    Mechanic/Linear/MechanicPositionSensor.hpp \
    Mechanic/Linear/MechanicMultiPortTranslationalMass.hpp \
    Mechanic/Linear/MechanicFreeLengthWall.hpp \
    Mechanic/Linear/MechanicForceTransformer.hpp \
    Mechanic/Linear/MechanicForceSensor.hpp \
    Mechanic/Linear/MechanicFixedPosition.hpp \
    Mechanic/Rotational/MechanicRotationalInertiaWithSingleGear.hpp \
    Mechanic/Rotational/MechanicRotationalInertiaWithGearRatio.hpp \
    Mechanic/Rotational/MechanicRotationalInertiaWithCoulumbFriction.hpp \
    Mechanic/Rotational/MechanicRotationalInertia.hpp \
    Mechanic/Linear/HopsanDefaultMechanicLinearComponents.h \
    Mechanic/Rotational/HopsanDefaultMechanicRotationalComponents.h \
    Connectivity/SignalOutputInterface.hpp \
    Connectivity/SignalInputInterface.hpp \
    Connectivity/MechanicInterfaceQ.hpp \
    Connectivity/MechanicInterfaceC.hpp \
    Connectivity/HydraulicInterfaceQ.hpp \
    Connectivity/HydraulicInterfaceC.hpp \
    Connectivity/HopsanDefaultconnectivityComponents.h \
    Signal/Arithmetics/HopsanDefaultSignalArithmeticComponents.h \
    Signal/Sources&Sinks/SignalTime.hpp \
    Signal/Sources&Sinks/SignalStepExponentialDelay.hpp \
    Signal/Sources&Sinks/SignalStep.hpp \
    Signal/Sources&Sinks/SignalStaircase.hpp \
    Signal/Sources&Sinks/SignalSquareWave.hpp \
    Signal/Sources&Sinks/SignalSoftStep.hpp \
    Signal/Sources&Sinks/SignalSink.hpp \
    Signal/Sources&Sinks/SignalSineWave.hpp \
    Signal/Sources&Sinks/SignalRamp.hpp \
    Signal/Sources&Sinks/SignalPulseWave.hpp \
    Signal/Sources&Sinks/SignalPulse.hpp \
    Signal/Sources&Sinks/SignalNoiseGenerator.hpp \
    Signal/SignalRouting/SignalDualRoute.hpp \
    Signal/SignalRouting/SignalQuadRoute.hpp \
    Signal/SignalRouting/SignalTripleRoute.hpp \
    Signal/Sources&Sinks/HopsanDefaultSignalSourcesAndSinks.h \
    Signal/SignalRouting/HopsanDefaultSignalRoutingComponents.h \
    Signal/Non-Linearities/HopsanDefaultSignalNonLinearities.h \
    Signal/Control/HopsanDefaultSignalControlComponents.h \
    Signal/Logic/HopsanDefaultSignalLogicComponents.h \
    Signal/Animation/HopsanDefaultSignalAnimationComponents.h \
    Signal/Filters/HopsanDefaultSignalTFComponents.h \
    Signal/Arithmetics/SignalSubtract.hpp \
    Special/Benchmarking/SignalDummy.hpp \
    Special/Benchmarking/HydraulicDummyQ.hpp \
    Special/Benchmarking/HydraulicDummyC.hpp \
    Special/SignalFFB/SignalFFBorIn.hpp \
    Special/SignalFFB/SignalFFBor.hpp \
    Special/SignalFFB/SignalFFBloopIn.hpp \
    Special/SignalFFB/SignalFFBloop.hpp \
    Special/SignalFFB/SignalFFBandIn.hpp \
    Special/SignalFFB/SignalFFBand.hpp \
    Special/SignalFFB/SignalFFB.hpp \
    Special/Benchmarking/HopsanDefaultBenchmarkingComponents.h \
    Special/SignalFFB/HopsanDefaultFBBComponents.h \
    Special/HopsanDefaultSpecialComponents.h \
    Special/MechanicVehicle1D.hpp \
    Hydraulic/MachineParts/MechanicSwashPlate.hpp \
    Hydraulic/MachineParts/MechanicCylinderBlockWithSwashPlate.hpp \
    Hydraulic/Valves/HydraulicOverCenterValve.hpp \
    Hydraulic/Valves/HydraulicOpenCenterValve.hpp \
    Hydraulic/Restrictors/HydraulicLossLessTConnector.hpp \
    Pneumatic/HopsanDefaultPneumaticComponents.h \
    Pneumatic/PneumaticVolume2.hpp \
    Pneumatic/PneumaticOrifice.hpp \
    Pneumatic/PneumaticQsrc.hpp \
    Pneumatic/PneumaticPTsrc.hpp \
    Pneumatic/PneumaticPsensor.hpp \
    Pneumatic/PneumaticTsensor.hpp \
    Pneumatic/PneumaticQmsensor.hpp \
    Pneumatic/PneumaticdEsensor.hpp \
    defaultComponentLibraryInternal.h \
    Signal/HopsanDefaultSignalComponents.h \
    Mechanic/HopsanDefaultMechanicComponents.h \
    Electric/HopsanDefaultElectricComponents.h \
    Connectivity/MechanicRotationalInterfaceQ.hpp \
    Connectivity/MechanicRotationalInterfaceC.hpp \
    Mechanic/Linear/MechanicPulley.hpp \
    Connectivity/PneumaticInterfaceQ.hpp \
    Connectivity/PneumaticInterfaceC.hpp \
    Connectivity/ElectricInterfaceQ.hpp \
    Connectivity/ElectricInterfaceC.hpp \
    defaultComponents.h \
    Mechanic/Linear/MechanicFixedPositionMultiPort.hpp \
    Mechanic/Rotational/MechanicAngularVelocityTransformer.hpp \
    Mechanic/Rotational/MechanicAngularVelocitySensor.hpp \
    Mechanic/Rotational/MechanicAngleSensor.hpp \
    Mechanic/Rotational/MechanicTorsionalSpring.hpp \
    Mechanic/Rotational/MechanicTorqueTransformer.hpp \
    Mechanic/Rotational/MechanicTorqueSensor.hpp \
    Mechanic/Rotational/MechanicThetaSource.hpp \
    Mechanic/Rotational/MechanicRotShaft.hpp \
    Mechanic/Rotational/MechanicJLink.hpp \
    Mechanic/Rotational/MechanicMotor.hpp \
    Mechanic/Rotational/MechanicRackAndPinion.hpp \
    Signal/Sources&Sinks/SignalConstant.hpp \
    Signal/Filters/SignalIntegrator2.hpp \
    Special/AeroComponents/AeroAircraft6DOF.hpp \
    Special/AeroComponents/SignalWaypoint.hpp \
    Special/AeroComponents/SignalTimeAccelerator.hpp \
    Special/AeroComponents/SignalStateMonitor.hpp \
    Special/AeroComponents/SignalEarthCoordinates.hpp \
    Special/AeroComponents/SignalAttitudeTVCcontrol.hpp \
    Special/AeroComponents/SignalAttitudeControl.hpp \
    Special/AeroComponents/AeroVehicleTVC.hpp \
    Special/AeroComponents/AeroPropeller.hpp \
    Special/AeroComponents/AeroJetEngine.hpp \
    Special/AeroComponents/AeroFuelTank.hpp \
    Special/AeroComponents/AeroAtmosphere.hpp \
    Special/AeroComponents/AeroAircraft6DOFS.hpp \
    Special/AeroComponents/FuelComponents/HydraulicFuelTankG.hpp \
    Special/AeroComponents/FuelComponents/HydraulicCentrifugalPumpJ.hpp \
    Special/AeroComponents/FuelComponents/HydraulicCentrifugalPump.hpp \
    Signal/Non-Linearities/SignalSaturation.hpp \
    Special/AeroComponents/HopsanDefaultAerocomponents.h \
    Hydraulic/MachineParts/HydraulicPumpPiston.hpp \
    Hydraulic/Valves/HydraulicPilotControlledCheckValve.hpp \
    Hydraulic/Valves/HydraulicPilotClosableCheckValve.hpp

SOURCES += \
    defaultComponentLibraryInternal.cc \
    defaultComponentLibrary.cc

OTHER_FILES += \
    Special/SignalFFB/HopsanDefaultFBBComponents.cci \
    Electric/HopsanDefaultElectricComponents.cci \
    Signal/Filters/hp1filter.svg \
    Signal/HopsanDefaultSignalComponents.cci
