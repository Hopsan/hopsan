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
#include "HopsanCore.h"

class HydraulicPressureSensor : public ComponentSignal
{
private:
    enum {P1,out};

public:
    static Component *Creator()
    {
        std::cout << "running HydraulicPressureSensor creator" << std::endl;
        return new HydraulicPressureSensor("DefaultPressureSensorName");
    }

    HydraulicPressureSensor(const string name,
                            const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
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
        double p = mPortPtrs[P1]->ReadNode(NodeHydraulic::PRESSURE);

        //Write new values to nodes
        mPortPtrs[out]->WriteNode(NodeSignal::VALUE, p);
    }
};

#endif // HYDRAULICPRESSURESENSOR_HPP_INCLUDED
