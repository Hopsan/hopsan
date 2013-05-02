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
//! @file   HydraulicPlugQ.hpp
//! @author FluMeS
//! @date   2013-05-02
//!
//! @brief Contains a Hydraulic Plug of Q-type
//!
//$Id$

#ifndef HYDRAULICPLUG_HPP_INCLUDED
#define HYDRAULICPLUG_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPlugQ : public ComponentQ
    {
    private:
        double *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicFlowSourceQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p, q, c, Zc;

            //Read variables from nodes
            c = (*mpND_c);
            Zc = (*mpND_Zc);

            //Flow source equations
            q = 0.0;
            p = c + q*Zc;

            if(p<0)
            {
                p=0;
            }

            (*mpND_p) = p;
            (*mpND_q) = q;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICPLUG_HPP_INCLUDED
