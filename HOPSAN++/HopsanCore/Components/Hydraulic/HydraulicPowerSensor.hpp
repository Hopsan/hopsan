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

class HydraulicPowerSensor : public ComponentSignal
{
private:
    enum {P1,out};

public:
    static Component *Creator()
    {
        std::cout << "running HydraulicPowerSensor creator" << std::endl;
        return new HydraulicPowerSensor("DefaultPowerSensorName");
    }

    HydraulicPowerSensor(const string name,
                         const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "HydraulicPowerSensor";

        addReadPort("P1", "NodeHydraulic", P1);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double p = mPortPtrs[P1]->readNode(NodeHydraulic::PRESSURE);
        double q = mPortPtrs[P1]->readNode(NodeHydraulic::MASSFLOW);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, p*q);
    }
};

#endif // HYDRAULICPOWERSENSOR_HPP_INCLUDED
