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
//! @file   MechanicRotationalInterfaceQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-30
//!
//! @brief Contains a rotational mechanic interface component of Q-type
//!
//$Id$

#ifndef MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED
#define MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInterfaceQ : public ComponentQ
    {

    private:
        Port *mpP1;
        double *mpND_t, *mpND_w, *mpND_c, *mpND_Zx;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInterfaceQ();
        }

        MechanicRotationalInterfaceQ() : ComponentQ()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        }

        void initialize()
        {
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::TORQUE);
            mpND_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CHARIMP);
        }

        void simulateOneTimestep()
        {
            //! @todo If this works, do same in other Q-type interface components

            //Calculate torque from c and Zx
            double w = (*mpND_w);
            double c = (*mpND_c);
            double Zx = (*mpND_Zx);

            (*mpND_t) = c + Zx*w;
        }
    };
}

#endif // MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED




