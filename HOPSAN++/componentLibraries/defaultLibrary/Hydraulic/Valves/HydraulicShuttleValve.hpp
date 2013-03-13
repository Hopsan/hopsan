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
//! @file   HydraulicShuttleValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-17
//!
//! @brief Contains a Shuttle Valve component
//!
//$Id$

#ifndef HYDRAULICSHUTTLEVALVE_HPP_INCLUDED
#define HYDRAULICSHUTTLEVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicShuttleValve : public ComponentQ
    {

    private:
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3, *mpND_out;

        Port *mpP1, *mpP2, *mpP3, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicShuttleValve();
        }

        void configure()
        {

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);
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

            mpND_p3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::PRESSURE);
            mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::FLOW);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::WAVEVARIABLE, 1e5);
            mpND_Zc3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::CHARIMP, 0);

            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            double p1, p2, p3, q1, q2, q3, c1, c2, c3, Zc1, Zc2, Zc3;

            //Get variable values from nodes
            p1 = (*mpND_p1);
            p2 = (*mpND_p2);
            c1 = (*mpND_c1);
            c2 = (*mpND_c2);
            c3 = (*mpND_c3);
            Zc1 = (*mpND_Zc1);
            Zc2 = (*mpND_Zc2);
            Zc3 = (*mpND_Zc3);

            //Shuttle valve equations
            if(p1>p2)
            {
                if(mpP3->isConnected()) { q3 = (c1-c3)/(Zc1+Zc3); }
                else { q3 = 0; }
                q1 = -q3;
                q2 = 0;
                (*mpND_out) = -1;
            }
            else
            {
                if(mpP3->isConnected()) { q3 = (c2-c3)/(Zc2+Zc3); }
                else { q3 = 0; }
                q2 = -q3;
                q1 = 0;
                (*mpND_out) = 1;
            }

            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;
            p3 = c3 + q3*Zc3;

            //Cavitation check
            if(p1 < 0.0)
            {
                p1 = 0.0;
            }
            if(p2 < 0.0)
            {
                p2 = 0.0;
            }
            if(p3 < 0.0)
            {
                p3 = 0.0;
            }

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_p3) = p3;
            (*mpND_q3) = q3;
        }
    };
}

#endif // HYDRAULICSHUTTLEVALVE_HPP_INCLUDED
