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
//! @file   HydraulicPressureReliefValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a hydraulic pressure relief valve with first order dynamics and signal and input signal
//! Written by Petter Krus 901015
//! Revised by Petter Krus 920324
//! Translated to HOPSAN NG by Robert Braun 100122
//$Id$

#ifndef HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED
#define HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief A Hydraulic Pressure Releife Valve
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureReliefValve : public ComponentQ
    {
    private:
        // Member variables
        TurbulentFlowFunction mTurb;
        ValveHysteresis mHyst;
        FirstOrderTransferFunction mFilterLP;
        double mPrevX0, x0, x0max;

        // Port and node data pointers
        Port *mpP1, *mpP2;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        double *mpPmax, *mpPh, *mpTao, *mpXv;

        // Constants
        double mKcs, mKcf, Cs, Cf, mQnom, mPnom;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureReliefValve();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic", "High pressure side");
            mpP2 = addPowerPort("P2", "NodeHydraulic", "Low pressure side");

            addOutputVariable("xv", "Equivalent spool position", "", &mpXv);

            addInputVariable("p_max", "Maximum opening pressure", "Pa", 20000000.0, &mpPmax);
            addInputVariable("tao", "Time Constant of Spool", "s", 0.01, &mpTao);
            addInputVariable("p_h", "Hysteresis Width", "Pa", 500000.0, &mpPh);

            addConstant("k_cs", "Steady State Characteristic due to Spring", "LeakageCoefficient", 0.00000001, mKcs);
            addConstant("k_cf", "Steady State Characteristic due to Flow Forces", "LeakageCoefficient", 0.00000001, mKcf);
            addConstant("q_nom", "Flow with Fully Open Valve and pressure drop Pnom", "m^3/s", 0.001, mQnom);
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

            mPnom = 7e6f;

            double tao = (*mpTao);

            x0max = mQnom / sqrt(mPnom);
            Cs = sqrt(mPnom) / mKcs;
            Cf = 1.0 / (mKcf*sqrt(mPnom));

            //x0 = 0.00001;
            mPrevX0 = 0.0;

            double wCutoff = 1 / tao;
            double num[2] = {1.0, 0.0};
            double den[2] = {1.0, 1.0/wCutoff};
            mFilterLP.initialize(mTimestep, num, den, mPrevX0, mPrevX0, 0.0, x0max);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;
            double b1, gamma, b2, xs, xh, xsh, wCutoff, pmax, ph, tao;
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
            pmax = (*mpPmax);
            ph = (*mpPh);
            tao = (*mpTao);

            //PRV Equations

            //Help variable b1
            b1 = Cs + (p1-p2)*Cf;
            if ( (p1-p2)*Cf < 0.0 )
            {
                b1 = Cs;
            }

            //Help variable gamma
            if (p1>p2)
            {
                if ( (sqrt(p1-p2)*2.0 + (Zc1+Zc2)*mPrevX0) != 0.0 )
                {
                    gamma = sqrt(p1-p2)*2.0 / (sqrt(p1-p2)*2.0 + (Zc1+Zc2)*mPrevX0);
                }
                else
                {
                    gamma = 1.0;
                }
            }
            else
            {
                if ( (sqrt(p2-p1)*2.0 + (Zc1+Zc2)*mPrevX0) != 0.0 )
                {
                    gamma = sqrt(p2-p1)*2.0 / (sqrt(p2-p1)*2.0 + (Zc1+Zc2)*mPrevX0);
                }
                else
                {
                    gamma = 1.0;
                }
            }

            //Help variable b2
            if (p1 > p2)
            {
                b2 = gamma*(Zc1+Zc2)*sqrt(p1-p2);
            }
            else
            {
                b2 = gamma*(Zc1+Zc2)*sqrt(p2-p1);
            }
            if (b2 < 0.0)
            {
                b2 = 0.0;
            }

            // Calculation of spool position
            xs = (gamma*(c1) + b2*mPrevX0/2.0 - pmax) / (b1+b2);

            //Hysteresis
            xh = ph / (b1+b2);                                  //Hysteresis width [m]
            xsh = mHyst.getValue(xs, xh, mPrevX0);

            //Filter
            wCutoff = (1.0 + b2/b1) * 1.0/tao;                //Cutoff frequency
            double num[2] = {1.0, 0.0};
            double den[2] = {1.0, 1.0/wCutoff};
            mFilterLP.setNumDen(num,den);
            double x0 = mFilterLP.update(xsh);

            //Turbulent flow equation
            mTurb.setFlowCoefficient(x0);
            q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
            q1 = -q2;
            p2 = c2+Zc2*q2;
            p1 = c1+Zc1*q1;

            // Cavitation
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
                xs = (c1 + b2*x0/2.0 - pmax) / (b1+b2);
                xsh = mHyst.getValue(xs, xh, mPrevX0);
                x0 = mFilterLP.value();        //! @todo Make the filter actually redo last step if possible; this will create an undesired delay of one iteration

                mTurb.setFlowCoefficient(x0);
                q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
                q1 = -q2;
                p2 = c2+Zc2*q2;
                p1 = c1+Zc1*q1;

                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            mPrevX0 = x0;

            //Write new variables to nodes
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpXv) = x0;
        }
    };
}

#endif // HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED
