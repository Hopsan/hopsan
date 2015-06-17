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
//! @file   HydraulicCheckValvePreLoaded.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-03-30
//!
//! @brief Contains a pre-loaded hydraulic check valve component
//!
//$Id$

#ifndef HYDRAULICCHECKVALVEPRELOADED_HPP_INCLUDED
#define HYDRAULICCHECKVALVEPRELOADED_HPP_INCLUDED

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
        TurbulentFlowFunction mQTurb;

        Port *mpP1, *mpP2;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        double *mpKs, *mpFs, *mpX;

    public:
        static Component *Creator()
        {
            return new HydraulicCheckValvePreLoaded();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addInputVariable("K_s", "Restrictor Coefficient", "", 5e-7, &mpKs);
            addInputVariable("F_s", "Spring Pre-Load Tension", "Pa", 0.0, &mpFs);

            addOutputVariable("x", "Position (for animation)", "", 0, &mpX);
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

            mQTurb.setFlowCoefficient(*mpKs);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, Ks, Fs, x;
            c1 = (*mpP1_c);
            Zc1 = (*mpP1_Zc);
            c2 = (*mpP2_c);
            Zc2 = (*mpP2_Zc);
            Ks = (*mpKs);
            Fs = (*mpFs);

            //Checkvalve equations
            mQTurb.setFlowCoefficient(Ks);
            if (c1 > c2+Fs)
            {
                q2 = mQTurb.getFlow(c1, c2, Zc1, Zc2);
                x = 1;
            }
            else
            {
                q2 = 0.0;
                x=0;
            }

            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;

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
            if (cav)
            {
                if (c1 > c2+Fs) { q2 = mQTurb.getFlow(c1, c2, Zc1, Zc2); }
                else { q2 = 0.0; }
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            //Write new values to nodes
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpX) = x;
        }
    };
}

#endif // HYDRAULICCHECKVALVEPRELOADED_HPP_INCLUDED
