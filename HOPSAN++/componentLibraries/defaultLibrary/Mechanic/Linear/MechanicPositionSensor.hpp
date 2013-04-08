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
            mpOut = addOutputVariable("out", "Position", "m");
        }


        void initialize()
        {
            mpND_x = getNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_out = getNodeDataPtr(mpOut, NodeSignal::Value);
            simulateOneTimestep(); //Set initial ouput node value
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_x);
        }
    };
}

#endif // MECHANICPOSITIONSENSOR_HPP_INCLUDED
