//!
//! @file   HydraulicPowerSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic Power Sensor Component
//!
//$Id$

#ifndef HYDRAULICPOWERSENSOR_HPP_INCLUDED
#define HYDRAULICPOWERSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicPowerSensor : public ComponentSignal
{
private:
    Port *mpP1, *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running HydraulicPowerSensor creator" << std::endl;
        return new HydraulicPowerSensor("PowerSensor");
    }

    HydraulicPowerSensor(const string name,
                         const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "HydraulicPowerSensor";

        mpP1 = addReadPort("P1", "NodeHydraulic");
        mpOut = addWritePort("out", "NodeSignal");
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double p = mpP1->readNode(NodeHydraulic::PRESSURE);
        double q = mpP1->readNode(NodeHydraulic::MASSFLOW);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, p*q);
    }
};

#endif // HYDRAULICPOWERSENSOR_HPP_INCLUDED
