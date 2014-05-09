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

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPlugQ : public ComponentQ
    {
    private:
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicPlugQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p, q, c, Zc;

            //Read variables from nodes
            c = (*mpP1_c);
            Zc = (*mpP1_Zc);

            //Flow source equations
            q = 0.0;
            p = c + q*Zc;

            if(p<0)
            {
                p=0;
            }

            (*mpP1_p) = p;
            (*mpP1_q) = q;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICPLUG_HPP_INCLUDED
