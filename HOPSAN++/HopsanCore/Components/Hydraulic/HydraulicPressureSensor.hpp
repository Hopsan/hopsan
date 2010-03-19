//!
//! @file   HydraulicPressureSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic Pressure Sensor Component
//!
//$Id$

#ifndef HYDRAULICPRESSURESENSOR_HPP_INCLUDED
#define HYDRAULICPRESSURESENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicPressureSensor : public ComponentSignal
{
private:
    Port *mpP1, * mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running HydraulicPressureSensor creator" << std::endl;
        return new HydraulicPressureSensor("PressureSensor");
    }

    HydraulicPressureSensor(const string name,
                            const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "HydraulicPressureSensor";

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

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, p);
    }
};

#endif // HYDRAULICPRESSURESENSOR_HPP_INCLUDED
