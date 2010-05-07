//!
//! @file   MechanicPositionSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Position Sensor Component
//!
//$Id$

#ifndef MECHANICPOSITIONSENSOR_HPP_INCLUDED
#define MECHANICPOSITIONSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicPositionSensor : public ComponentSignal
{
private:
    Port *mpP1, *mpOut;


public:
    static Component *Creator()
    {
        return new MechanicPositionSensor("PositionSensor");
    }

    MechanicPositionSensor(const string name) : ComponentSignal(name)
    {
        mTypeName = "MechanicPositionSensor";

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
        double x = mpP1->readNode(NodeMechanic::POSITION);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, x);
    }
};

#endif // MECHANICPOSITIONSENSOR_HPP_INCLUDED
