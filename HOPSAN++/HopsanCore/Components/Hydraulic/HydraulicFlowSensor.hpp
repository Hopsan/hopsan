//!
//! @file   HydraulicFlowSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic Flow Sensor Component
//!
//$Id$

#ifndef HYDRAULICFLOWSENSOR_HPP_INCLUDED
#define HYDRAULICFLOWSENSOR_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicFlowSensor : public ComponentSignal
{
private:
    enum {P1,out};

public:
    static Component *Creator()
    {
        std::cout << "running HydraulicFlowSensor creator" << std::endl;
        return new HydraulicFlowSensor("DefaultFlowSensorName");
    }

    HydraulicFlowSensor(const string name,
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
        double q = mPortPtrs[P1]->ReadNode(NodeHydraulic::MASSFLOW);

        //Write new values to nodes
        mPortPtrs[out]->WriteNode(NodeSignal::VALUE, q);
    }
};

#endif // HYDRAULICFLOWSENSOR_HPP_INCLUDED
