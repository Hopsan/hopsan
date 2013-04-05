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
//! @file   MechanicAngleSensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-12
//!
//! @brief Contains a Mechanic Angle Sensor Component
//!
//$Id$

#ifndef MECHANICANGLESENSOR_HPP_INCLUDED
#define MECHANICANGLESENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngleSensor : public ComponentSignal
    {
    private:
        double *mpND_phi, *mpND_out;
        Port *mpP1, *mpOut;


    public:
        static Component *Creator()
        {
            return new MechanicAngleSensor();
        }

        void configure()
        {

            mpP1 = addReadPort("P1", "NodeMechanicRotational");
            mpOut = addWriteVariable("out", "Angle", "rad");
        }


        void initialize()
        {
            mpND_phi = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_phi);
        }
    };
}

#endif // MECHANICANGLESENSOR_HPP_INCLUDED
