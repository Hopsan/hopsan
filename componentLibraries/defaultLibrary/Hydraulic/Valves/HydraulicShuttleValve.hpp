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
