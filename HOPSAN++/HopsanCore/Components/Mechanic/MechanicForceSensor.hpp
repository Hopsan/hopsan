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
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicForceSensor : public ComponentSignal
{
private:
    Port *mpP1, *mpOut;

public:
    static Component *Creator()
    {
        //std::cout << "running MechanicForceSensor creator" << std::endl;
        return new MechanicForceSensor("ForceSensor");
    }

    MechanicForceSensor(const string name,
                        const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "MechanicForceSensor";

        mpP1 = addReadPort("P1", "NodeMechanic");
        mpOut = addWritePort("out", "NodeSignal");
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double f = mpP1->readNode(NodeMechanic::FORCE);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, f);
    }
};

#endif // MECHANICFORCESENSOR_HPP_INCLUDED
