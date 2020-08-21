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
//! @file   mechanicRotPowerSensor.hpp
//! @author Viktor Larsson <viktor.larsson@liu.se>
//! @date   2020-08-19
//!
//! @brief Contains a Mechanic Rotational Power Sensor Component
//!


#ifndef MECHANICROTATIONALPOWERSENSOR_HPP_INCLUDED
#define MECHANICROTATIONALPOWERSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!

    //!
    class MechanicRotationalPowerSensor : public ComponentSignal
    {
    private:
        double *mpND_t, *mpND_out,*mpND_w, *mpPmax;
        Port *mpP1;


    public:
        static Component *Creator()
        {
            return new MechanicRotationalPowerSensor();
        }

        void configure()
        {
            mpP1 = addReadPort("P1", "NodeMechanicRotational", "", Port::NotRequired);
            addOutputVariable("out","Power","W",&mpND_out);
            addInputVariable("PMax","Max power (animation)","W",1000,&mpPmax);
        }


        void initialize()
        {
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_w)*(*mpND_t);
        }
    };
}

#endif // MECHANICROTPOWERSENSOR
