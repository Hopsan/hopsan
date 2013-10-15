/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   defaultComponentLibraryInternal.cc
//! @author FluMeS
//! @date   2012-01-13
//! @brief Contains the register_default_components function that registers all built in components
//!
//$Id$

#include "defaultComponentLibraryInternal.h"
#include "defaultComponents.h"
//#include "signal.h"
#include <stdlib.h>

//! @defgroup Components Components
//!
//! @defgroup HydraulicComponents HydraulicComponents
//! @ingroup Components
//!
//! @defgroup MechanicalComponents MechanicalComponents
//! @ingroup Components
//!
//! @defgroup SignalComponents SignalComponents
//! @ingroup Components
//!
//! @defgroup ElectricComponents ElectricComponents
//! @ingroup Components



using namespace hopsan;

////Uncomment this and the Repoint SEGFAULT thing below to catch segfaults
////vvv The function to be run at SEGFAULT, see below. (http://www.cplusplus.com/reference/clibrary/csignal/signal/)
//void terminate (int /*param*/)
//{
//    std::cout << "Terminating program because of SEGFAULT, putz..." << std::endl;
//    exit(1);
//    //Maybe have a global message tracker to write ino in on whats going on that could be used to ptint here?!
//}
////^^^

//!
//! @brief Registers the creator function of all built in components
//! @param [in,out] pComponentFactory A pointer the the component factory in wich to register the components
//!
void hopsan::register_default_components(ComponentFactory* pComponentFactory)
{
//    //vvv Repoint SEGFAULT to the terminate function, could be used to tell component makers info about their fault. (http://www.cplusplus.com/reference/clibrary/csignal/signal/)
//    void (*prev_fn)(int);
//    prev_fn = signal (SIGSEGV,terminate);
//    //^^^


    // ========== Special Components ==========
    #include "Special/AeroComponents/HopsanDefaultAerocomponents.cci"
    #include "Special/SignalFFB/HopsanDefaultFBBComponents.cci"

    // ========== Hydraulic components ==========
    // ----- Hydraulic Valves -----
    pComponentFactory->registerCreatorFunction("HydraulicCheckValve", HydraulicCheckValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCheckValveWithOrifice", HydraulicCheckValveWithOrifice::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic22DirectionalValve", Hydraulic22DirectionalValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic22Valve", Hydraulic22Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic22PoppetValve", Hydraulic22PoppetValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic32DirectionalValve", Hydraulic32DirectionalValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic33Valve", Hydraulic33Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic33ShuttleValve", Hydraulic33ShuttleValve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic42Valve", Hydraulic42Valve::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43Valve", Hydraulic43Valve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicValve416", HydraulicValve416::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43ValveNeutralToTank", Hydraulic43ValveNeutralToTank::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43ValveNeutralSupplyToTank", Hydraulic43ValveNeutralSupplyToTank::Creator);
    pComponentFactory->registerCreatorFunction("Hydraulic43LoadSensingValve", Hydraulic43LoadSensingValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureReliefValve", HydraulicPressureReliefValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureReliefValveG", HydraulicPressureReliefValveG::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureReducingValve", HydraulicPressureReducingValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureReducingValveG", HydraulicPressureReducingValveG::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureDropValve", HydraulicPressureDropValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureControlledValve", HydraulicPressureControlledValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureControlValveG", HydraulicPressureControlValveG::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureCompensatingValve", HydraulicPressureCompensatingValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureCompensatingValveG", HydraulicPressureCompensatingValveG::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCounterBalanceValveG", HydraulicCounterBalanceValveG::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicValve63OC", HydraulicValve63OC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicValve43LS", HydraulicValve43LS::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicValve43", HydraulicValve43::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicValve33", HydraulicValve33::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicMotorJload", HydraulicMotorJload::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPistonMload", HydraulicPistonMload::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPistonMkload", HydraulicPistonMkload::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicShuttleValve", HydraulicShuttleValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCheckValvePreLoaded", HydraulicCheckValvePreLoaded::Creator);

    // ----- Other Hydraulic Components -----
    pComponentFactory->registerCreatorFunction("HydraulicLaminarOrifice", HydraulicLaminarOrifice::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTurbulentOrifice", HydraulicTurbulentOrifice::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVolume", HydraulicVolume::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureSourceC", HydraulicPressureSourceC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicMultiPressureSourceC", HydraulicMultiPressureSourceC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFlowSourceQ", HydraulicFlowSourceQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureSourceQ", HydraulicPressureSourceQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFixedDisplacementPump", HydraulicFixedDisplacementPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicOpenCenterValve", HydraulicOpenCenterValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVariableDisplacementPump", HydraulicVariableDisplacementPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicAckumulator", HydraulicAckumulator::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureSensor", HydraulicPressureSensor::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFlowSensor", HydraulicFlowSensor::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPowerSensor", HydraulicPowerSensor::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCylinderC", HydraulicCylinderC::Creator);
    //pComponentFactory->registerCreatorFunction("HydraulicCylinderCMulti", HydraulicCylinderCMulti::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCylinderQ", HydraulicCylinderQ::Creator);
//    pComponentFactory->registerCreatorFunction("HydraulicPistonMload", HydraulicCylinderQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTLMlossless", HydraulicTLMlossless::Creator);
    //pComponentFactory->registerCreatorFunction("HydraulicSubSysExample", HydraulicSubSysExample::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicTankC", HydraulicTankC::Creator);
    //pComponentFactory->registerCreatorFunction("HydraulicTankQ", HydraulicTankQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFixedDisplacementMotorQ", HydraulicFixedDisplacementMotorQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVariableDisplacementMotorQ", HydraulicVariableDisplacementMotorQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicVolumeMultiPort", HydraulicVolumeMultiPort::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicLosslessConnector", HydraulicLosslessConnector::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicLosslessTConnector", HydraulicLosslessTConnector::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicMachineC", HydraulicMachineC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPressureControlledPump", HydraulicPressureControlledPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicHose", HydraulicHose::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicDummyC", HydraulicDummyC::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicDummyQ", HydraulicDummyQ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPilotControlledCheckValve", HydraulicPilotControlledCheckValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPilotClosableCheckValve", HydraulicPilotClosableCheckValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicMultiTankC", HydraulicMultiTankC::Creator);

    pComponentFactory->registerCreatorFunction("HydraulicOverCenterValve", HydraulicOverCenterValve::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicValvePlate", HydraulicValvePlate::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPumpPiston", HydraulicPumpPiston::Creator);

    pComponentFactory->registerCreatorFunction("HydraulicCentrifugalPump", HydraulicCentrifugalPump::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicCentrifugalPumpJ", HydraulicCentrifugalPumpJ::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicFuelTankG", HydraulicFuelTankG::Creator);
    pComponentFactory->registerCreatorFunction("HydraulicPlugQ", HydraulicPlugQ::Creator);

    //Signal components
    #include "Signal/HopsanDefaultSignalComponents.cci"


    //Mechanical components
    pComponentFactory->registerCreatorFunction("MechanicForceTransformer", MechanicForceTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicVelocityTransformer", MechanicVelocityTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTorqueTransformer", MechanicTorqueTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicAngularVelocityTransformer", MechanicAngularVelocityTransformer::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalMass", MechanicTranslationalMass::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalMassWithLever", MechanicTranslationalMassWithLever::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalMassWithCoulombFriction", MechanicTranslationalMassWithCoulombFriction::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalSpring", MechanicTranslationalSpring::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTorsionalSpring", MechanicTorsionalSpring::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotShaft", MechanicRotShaft::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertia", MechanicRotationalInertia::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertiaMultiPort", MechanicRotationalInertiaMultiPort::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertiaWithGearRatio", MechanicRotationalInertiaWithGearRatio::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertiaWithSingleGear", MechanicRotationalInertiaWithSingleGear::Creator);
    pComponentFactory->registerCreatorFunction("MechanicSpeedSensor", MechanicSpeedSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicForceSensor", MechanicForceSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicPositionSensor", MechanicPositionSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicAngleSensor", MechanicAngleSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTranslationalLosslessConnector", MechanicTranslationalLosslessConnector::Creator);
    pComponentFactory->registerCreatorFunction("MechanicAngularVelocitySensor", MechanicAngularVelocitySensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicTorqueSensor", MechanicTorqueSensor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicMultiPortTranslationalMass", MechanicMultiPortTranslationalMass::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRackAndPinion", MechanicRackAndPinion::Creator);
    pComponentFactory->registerCreatorFunction("MechanicJLink", MechanicJLink::Creator);
    pComponentFactory->registerCreatorFunction("MechanicJLink2", MechanicJLink2::Creator);
    pComponentFactory->registerCreatorFunction("MechanicThetaSource", MechanicThetaSource::Creator);
    pComponentFactory->registerCreatorFunction("MechanicVehicle1D", MechanicVehicle1D::Creator);
    pComponentFactory->registerCreatorFunction("MechanicRotationalInertiaWithCoulumbFriction", MechanicRotationalInertiaWithCoulombFriction::Creator);
    pComponentFactory->registerCreatorFunction("MechanicFixedPosition", MechanicFixedPosition::Creator);
    pComponentFactory->registerCreatorFunction("MechanicFixedPositionMultiPort", MechanicFixedPositionMultiPort::Creator);
    pComponentFactory->registerCreatorFunction("MechanicFreeLengthWall", MechanicFreeLengthWall::Creator);
    pComponentFactory->registerCreatorFunction("MechanicCylinderBlockWithSwashPlate", MechanicCylinderBlockWithSwashPlate::Creator);
    pComponentFactory->registerCreatorFunction("MechanicSwashPlate", MechanicSwashPlate::Creator);
    pComponentFactory->registerCreatorFunction("MechanicMotor", MechanicMotor::Creator);
    pComponentFactory->registerCreatorFunction("MechanicPulley", MechanicPulley::Creator);

    pComponentFactory->registerCreatorFunction("CombustionEngine", CombustionEngine::Creator);


    //Electric components
    #include "Electric/HopsanDefaultElectricComponents.cci"

    //Pneumatic components
    #include "Pneumatic/HopsanDefaultPneumaticComponents.cci"

    //Connectivity components
    #include "Connectivity/HopsanDefaultConnectivityComponents.cci"

    // Special component


}
