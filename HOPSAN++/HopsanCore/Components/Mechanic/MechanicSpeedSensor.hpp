//!
//! @file   MechanicSpeedSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Speed Sensor Component
//!
//$Id$

#ifndef MECHANICSPEEDSENSOR_HPP_INCLUDED
#define MECHANICSPEEDSENSOR_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class MechanicSpeedSensor : public ComponentSignal
{
private:
    enum {P1,out};

public:
    static Component *Creator()
    {
        std::cout << "running MechanicSpeedSensor creator" << std::endl;
        return new MechanicSpeedSensor("DefaultSpeedSensorName");
    }

    MechanicSpeedSensor(const string name,
                        const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "MechanicSpeedSensor";

        addReadPort("P1", "NodeMechanic", P1);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double v = mPortPtrs[P1]->readNode(NodeMechanic::VELOCITY);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, v);
    }
};

#endif // MECHANICSPEEDSENSOR_HPP_INCLUDED
