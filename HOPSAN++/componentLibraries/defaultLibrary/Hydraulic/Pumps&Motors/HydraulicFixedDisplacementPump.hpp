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
//! @file   HydraulicFixedDisplacementPump.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-07
//!
//! @brief Contains a hydraulic pump with fixed displacement component
//!
//$Id$

#ifndef HYDRAULICFIXEDDISPLACEMENTPUMP_HPP_INCLUDED
#define HYDRAULICFIXEDDISPLACEMENTPUMP_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicFixedDisplacementPump : public ComponentQ
    {
    private:
        double n;             // rad/s
        double dp;
        double Kcp;
        double a;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_a;

        Port *mpP1, *mpP2, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicFixedDisplacementPump();
        }

        void configure()
        {
            n = 250.0;
            dp = 0.00005;
            Kcp = 0;
            a=0;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpOut = addWritePort("a", "NodeSignal", Port::NotRequired);

            registerParameter("n_p", "Angular Velocity", "[rad/s]", n);
            registerParameter("D_p", "Displacement", "[m^3/rev]", dp);
            registerParameter("C_lp", "Leakage Coefficient", "[(m^3/s)/Pa]", Kcp);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            mpND_a = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_a) = 0;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;
            bool cav = false;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);

            //Fixed Displacement Pump equations
            q2 = ( dp*n/(2.0*pi) + Kcp*(c1-c2) ) / ( (Zc1+Zc2)*Kcp+1 );
            q1 = -q2;
            p2 = c2 + Zc2*q2;
            p1 = c1 + Zc1*q1;

            /* Cavitation Check */
            if (p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if (p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if (cav)
            {
                q2 = ( dp*n/(2.0*pi) + Kcp*(c1-c2) ) / ( (Zc1+Zc2)*Kcp+1 );

                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 <= 0.0)
                {
                    p1 = 0.0;
                    q2 = std::min(q2, 0.0);
                }
                if (p2 <= 0.0)
                {
                    p2 = 0.0;
                    q2 = std::max(q2, 0.0);
                }
                q1 = -q2;
            }

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_a) += n/mTimestep;
        }
    };
}

#endif // HYDRAULICFIXEDDISPLACEMENTPUMP_HPP_INCLUDED
