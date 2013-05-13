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
        double me;
        Port *mpPm1;
        double *mpND_f, *mpND_x, *mpND_v, *mpND_c, *mpND_me;

    public:
        static Component *Creator()
        {
            return new MechanicFixedPosition();
        }

        void configure()
        {
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");
            addConstant("m_e", "Equivalent Mass", "[kg]", 1, me);
        }

        void initialize()
        {
            mpND_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
            mpND_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
            mpND_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
            mpND_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
            mpND_me = getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);

            (*mpND_v) = 0;
            (*mpND_me) = me;
        }


        void simulateOneTimestep()
        {
            //Equations
            (*mpND_f) = (*mpND_c);
        }
    };
}

#endif // MECHANICFIXEDPOSITION_HPP_INCLUDED
