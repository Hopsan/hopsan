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
        Port *mpP1, *mpP2, *mpP3, *mpOut;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc;
        double *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        double *mpP3_p, *mpP3_q, *mpP3_c, *mpP3_Zc, *mpOut_v;


    public:
        static Component *Creator()
        {
            return new HydraulicShuttleValve();
        }

        void configure()
        {

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic");
            mpOut = addOutputVariable("out", "", "");
        }

        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            mpP3_p = getSafeNodeDataPtr(mpP3, NodeHydraulic::Pressure);
            mpP3_q = getSafeNodeDataPtr(mpP3, NodeHydraulic::Flow);
            mpP3_c = getSafeNodeDataPtr(mpP3, NodeHydraulic::WaveVariable);
            mpP3_Zc = getSafeNodeDataPtr(mpP3, NodeHydraulic::CharImpedance);

            mpOut_v = getSafeNodeDataPtr(mpOut, NodeSignal::Value);
        }


        void simulateOneTimestep()
        {
            double p1, p2, p3, q1, q2, q3, c1, c2, c3, Zc1, Zc2, Zc3;

            //Get variable values from nodes
            p1 = (*mpP1_p);
            p2 = (*mpP2_p);
            c1 = (*mpP1_c);
            c2 = (*mpP2_c);
            c3 = (*mpP3_c);
            Zc1 = (*mpP1_Zc);
            Zc2 = (*mpP2_Zc);
            Zc3 = (*mpP3_Zc);

            //Shuttle valve equations
            if(p1>p2)
            {
//                if(mpP3->isConnected()) { q3 = (c1-c3)/(Zc1+Zc3); }
//                else { q3 = 0; }
                q3 = (c1-c3)/(Zc1+Zc3);
                q1 = -q3;
                q2 = 0;
                (*mpOut_v) = -1;
            }
            else
            {
//                if(mpP3->isConnected()) { q3 = (c2-c3)/(Zc2+Zc3); }
//                else { q3 = 0; }
                q3 = (c2-c3)/(Zc2+Zc3);
                q2 = -q3;
                q1 = 0;
                (*mpOut_v) = 1;
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
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpP3_p) = p3;
            (*mpP3_q) = q3;
        }
    };
}

#endif // HYDRAULICSHUTTLEVALVE_HPP_INCLUDED
