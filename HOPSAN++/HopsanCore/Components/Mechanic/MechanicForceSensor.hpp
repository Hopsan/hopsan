//!
//! @file   MechanicSpeedSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Speed Sensor Component
//!
//$Id$

#ifndef MECHANICFORCESENSOR_HPP_INCLUDED
#define MECHANICFORCESENSOR_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class MechanicForceSensor : public ComponentSignal
{
private:
    enum {P1,out};

public:
    static Component *Creator()
    {
        std::cout << "running MechanicForceSensor creator" << std::endl;
        return new MechanicForceSensor("DefaultForceSensorName");
    }

    MechanicForceSensor(const string name,
                        const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
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
        double f = mPortPtrs[P1]->readNode(NodeMechanic::FORCE);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, f);
    }
};

#endif // MECHANICFORCESENSOR_HPP_INCLUDED
