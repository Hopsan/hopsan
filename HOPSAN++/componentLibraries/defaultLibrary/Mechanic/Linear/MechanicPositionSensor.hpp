/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicPositionSensor : public ComponentSignal
    {
    private:
        double *mpND_x, *mpND_out;
        Port *mpP1, *mpOut;


    public:
        static Component *Creator()
        {
            return new MechanicPositionSensor();
        }

        void configure()
        {

            mpP1 = addReadPort("P1", "NodeMechanic", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);
            disableStartValue(mpOut, NodeSignal::Value);
        }


        void initialize()
        {
            mpND_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value);
            mpOut->setSignalNodeUnitAndDescription("m","Position");
            simulateOneTimestep(); //Set initial ouput node value
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_x);
        }
    };
}

#endif // MECHANICPOSITIONSENSOR_HPP_INCLUDED
