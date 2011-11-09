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
//! @file   MechanicFixedPosition.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-11-09
//!
//! @brief Contains a Mechanic Fixed Position Component
//!
//$Id$

#ifndef MECHANICFIXEDPOSITION_HPP_INCLUDED
#define MECHANICFIXEDPOSITION_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class MechanicFixedPosition : public ComponentQ
    {

    private:
        Port *mpPm1;
        double *mpND_f, *mpND_x, *mpND_v, *mpND_c;

    public:
        static Component *Creator()
        {
            return new MechanicFixedPosition();
        }

        MechanicFixedPosition() : ComponentQ()
        {
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");
        }

        void initialize()
        {
            mpND_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::FORCE);
            mpND_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::POSITION);
            mpND_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::VELOCITY);
            mpND_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WAVEVARIABLE);

            (*mpND_x) = 0;
            (*mpND_v) = 0;
        }


        void simulateOneTimestep()
        {
            //Equations
            (*mpND_f) = (*mpND_c);
        }
    };
}

#endif // MECHANICFIXEDPOSITION_HPP_INCLUDED
