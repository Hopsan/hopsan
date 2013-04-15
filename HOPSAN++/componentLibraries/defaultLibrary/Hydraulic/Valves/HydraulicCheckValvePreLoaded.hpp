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
//! @file   HydraulicCheckValvePreLoaded.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-03-30
//!
//! @brief Contains a pre-loaded hydraulic check valve component
//!
//$Id: HydraulicCheckValve.hpp 3808 2012-01-05 13:34:11Z petno25 $

#ifndef HYDRAULICCHECKVALVEPRELOADED_HPP_INCLUDED
#define HYDRAULICCHECKVALVEPRELOADED_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicCheckValvePreLoaded : public ComponentQ
    {
    private:
        double *mpKs, *mpFs;
        bool cav;
        TurbulentFlowFunction qTurb_;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicCheckValvePreLoaded();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addInputVariable("K_s", "Restrictor Coefficient", "[]", 0.000000025, &mpKs);
            addInputVariable("F_s", "Spring Pre-Load Tension", "[Pa]", 0.0, &mpFs);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            qTurb_.setFlowCoefficient(*mpKs);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, Ks, Fs;
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            Ks = (*mpKs);
            Fs = (*mpFs);

            //Checkvalve equations
            qTurb_.setFlowCoefficient(Ks);
            if (c1 > c2+Fs) { q2 = qTurb_.getFlow(c1, c2, Zc1, Zc2); }
            else { q2 = 0.0; }

            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;

            //Cavitation check
            cav = false;
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
                if (c1 > c2+Fs) { q2 = qTurb_.getFlow(c1, c2, Zc1, Zc2); }
                else { q2 = 0.0; }
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICCHECKVALVEPRELOADED_HPP_INCLUDED
