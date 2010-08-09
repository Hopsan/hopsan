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
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicSpeedSensor : public ComponentSignal
{
private:
    Port *mpP1, *mpOut;

public:
    static Component *Creator()
    {
        return new MechanicSpeedSensor("SpeedSensor");
    }

    MechanicSpeedSensor(const std::string name) : ComponentSignal(name)
    {
        mTypeName = "MechanicSpeedSensor";

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
        double v = mpP1->readNode(NodeMechanic::VELOCITY);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, v);
    }
};

#endif // MECHANICSPEEDSENSOR_HPP_INCLUDED
