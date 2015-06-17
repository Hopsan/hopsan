/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicPilotClosableCheckValve.hpp
//! @author Isak Demir
//! @date   2011-05-10
//!
//! @brief Contains a Pilot Closable Hydraulic Checkvalve component
//!
//$Id$

#ifndef HYDRAULICPILOTCLOSABLECHECKVALVE_HPP_INCLUDED
#define HYDRAULICPILOTCLOSABLECHECKVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPilotClosableCheckValve : public ComponentQ
    {
    private:
        // Member variables
        TurbulentFlowFunction mQTurb;

        // Port and node data pointers
        Port *mpP1, *mpP2, *mpPPilot;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc,
        *mpPPilot_p, *mpPPilot_c, *mpX;

        // Constants
        double mKs;

    public:
        static Component *Creator()
        {
            return new HydraulicPilotClosableCheckValve();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpPPilot = addPowerPort("P_PILOT", "NodeHydraulic");

            addOutputVariable("x", "Position (for animation)", "", 0, &mpX);

            addConstant("K_s", "Restrictor Coefficient", "", 5e-7, mKs);
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

            mpPPilot_p = getSafeNodeDataPtr(mpPPilot, NodeHydraulic::Pressure);
            mpPPilot_c = getSafeNodeDataPtr(mpPPilot, NodeHydraulic::WaveVariable);

            mQTurb.setFlowCoefficient(mKs);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, p_pilot, c_pilot, x;

            //Get variable values from nodes
            c1 = (*mpP1_c);
            Zc1 = (*mpP1_Zc);
            c2 = (*mpP2_c);
            Zc2 = (*mpP2_Zc);
            c_pilot = (*mpPPilot_c);

            //Checkvalve equations
            if (c1 > (c2 + c_pilot))
            {
                q2 = mQTurb.getFlow(c1, c2, Zc1, Zc2);
                x=1;
            }
            else
            {
                q2 = 0.0;
                x=0;
            }

            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;
            p_pilot = c_pilot;

            //Cavitation check
            bool cav = false;
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
            if (p_pilot < 0.0)
            {
                c_pilot = 0;
                cav = true;
            }
            if (cav)
            {
                if (c1 > c2) { q2 = mQTurb.getFlow(c1, c2, Zc1, Zc2); }
                else { q2 = 0.0; }
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                p_pilot = c_pilot;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            //Write new values to nodes
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpPPilot_p) = p_pilot;
            (*mpX) = x;
        }
    };
}

#endif // HYDRAULICPILOTCONTROLLEDCHECKVALVE_HPP_INCLUDED
