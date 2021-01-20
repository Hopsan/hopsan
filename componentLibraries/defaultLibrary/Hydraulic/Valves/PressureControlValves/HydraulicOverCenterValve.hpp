/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicOverCenterValve.hpp
//! @author Mikael Axin <mikael.axin@liu.se>
//! @date   2010-01-13
//!
//! @brief Contains a hydraulic over center valve with first order dynamics
//!
//$Id$

#ifndef HYDRAULICOVERCENTERVALVE_HPP_INCLUDED
#define HYDRAULICOVERCENTERVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOverCenterValve : public ComponentQ
    {
    private:
        // Member variables
        TurbulentFlowFunction mTurb;
        ValveHysteresis mHyst;
        FirstOrderTransferFunction mFilterLP;
        double mPrevX0, mCs, mCf;

        // Port and node data pointers
        Port *mpP1, *mpP2, *mpPControl;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc,
               *mpPControl_p, *mpPControl_c;
        double *mpPref, *mpPh, *mpArat, *mpXv;

        // Constants
        double mTao, mKcs, mKcf, mPnom, mQnom;

    public:
        static Component *Creator()
        {
            return new HydraulicOverCenterValve();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpPControl = addPowerPort("P_CONTROL", "NodeHydraulic");

            addInputVariable("p_ref", "Reference Opening Pressure", "Pa", 2000000, &mpPref);
            addInputVariable("p_h", "Hysteresis Width", "Pa", 500000, &mpPh);
            addInputVariable("a_ratio", "Area ratio", "-", 5.0, &mpArat);

            addOutputVariable("xv", "Equivalent spool position", "", &mpXv);

            addConstant("tao", "Time Constant of Spool", "s", 0.01, mTao);
            addConstant("k_cs", "Steady State Characteristic due to Spring", "LeakageCoefficient", 0.00000001, mKcs);
            addConstant("k_cf", "Steady State Characteristic due to Flow Forces", "LeakageCoefficient", 0.00000001, mKcf);
            addConstant("q_nom", "Flow with Fully Open Valve and pressure drop p_nom", "m^3/s", 0.001, mQnom);
            addConstant("p_nom", "Nominal pressure drop", "Pa", 7e6, mPnom);
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

            mpPControl_p = getSafeNodeDataPtr(mpPControl, NodeHydraulic::Pressure);
            mpPControl_c = getSafeNodeDataPtr(mpPControl, NodeHydraulic::WaveVariable);

            mPrevX0 = 0.0;
            mCs = sqrt(mPnom)/mKcs;
            mCf = 1/(mKcf * sqrt(mPnom));
            double x0max = mQnom/sqrt(mPnom);

            double wCutoff = 1 / mTao;
            double num[2] = {1.0, 0.0};
            double den[2] = {1.0, 1.0/wCutoff};
            mFilterLP.initialize(mTimestep, num, den, mPrevX0, mPrevX0, 0, x0max);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, p_control, c_control;
            double b1, xs, xh, xsh, pref, ph, arat;
            bool cav = false;

            //Get variable values from nodes
            p1 = (*mpP1_p);
            q1 = (*mpP1_q);
            c1 = (*mpP1_c);
            Zc1 = (*mpP1_Zc);
            p2 = (*mpP2_p);
            q2 = (*mpP2_q);
            c2 = (*mpP2_c);
            Zc2 = (*mpP2_Zc);
            p_control = (*mpPControl_p);
            c_control = (*mpPControl_c);
            pref = (*mpPref);
            ph = (*mpPh);
            arat = (*mpArat);

            /* Equations */

            b1 = mCs+mCf*(p1-p2);                               // Help Variable, equals sqrt(p1-p2)/Kctot
            xs = (p1 - pref + arat * p_control) / b1;           // Spool position calculation
            xh = ph/b1;
            xsh = mHyst.getValue(xs, xh, mPrevX0);              // Hysteresis
            double x0 = mFilterLP.update(xsh);                  // Dynamics
            mTurb.setFlowCoefficient(x0);                       // Turbulent Flow Calculation
            q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
            q1 = -q2;

            p1 = c1 + Zc1 * q1;                                 // Pressure Calulation
            p2 = c2 + Zc2 * q2;
            p_control = c_control;

            p_control = std::max(0.0, p_control);

            if (p1 < 0.0)                                       // Check for cavitation
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
            if (cav)                                            // Cavitatiaon, redo calculations with new c and Zc
            {
                q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            mPrevX0 = x0;

            (*mpP1_p) = p1;                                    // Write new values to nodes
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpPControl_p) = p_control;
            (*mpXv) = x0;
        }
    };
}

#endif // HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED

