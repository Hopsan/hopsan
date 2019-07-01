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
//! @file   HydraulicFixedDisplacementPump.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contains a hydraulic motor component with inertia load
//!
//$Id$

#ifndef HYDRAULICFIXEDDISPLACEMENTMOTORQ_H
#define HYDRAULICFIXEDDISPLACEMENTMOTORQ_H


#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicFixedDisplacementMotorQ : public ComponentQ
    {
    private:
        double *mpDm, *mpBm, *mpClm, *mpJ;
        //Integrator mFirstIntegrator;
        //Integrator mSecondIntegrator;
        DoubleIntegratorWithDamping mIntegrator;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_t3, *mpND_a3, *mpND_w3, *mpND_c3, *mpND_Zx3;

        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicFixedDisplacementMotorQ();
        }

        void configure()
        {
            addInputVariable("D_m", "Displacement", "m^3/rev", 0.00005, &mpDm);
            addInputVariable("B_m", "Viscous Friction", "Nm/rad", 0.0, &mpBm);
            addInputVariable("C_lm", "Leakage Coefficient", "", 0.0, &mpClm);
            addInputVariable("J_m", "Inertia Load", "kg*m^2", 0.1, &mpJ);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");
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

            mpND_t3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Torque);
            mpND_a3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Angle);
            mpND_w3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::AngularVelocity);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::WaveVariable);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::CharImpedance);

            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, t3, a3, w3, c3, Zx3;
            double dpe, ble, gamma, c1a, c2a, ct, q1a, q2a, q1leak, q2leak;
            double prevA3;

            double d = (*mpDm);
            double B = (*mpBm);
            double cl = (*mpClm);
            double J = (*mpJ);

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            c3 = (*mpND_c3);
            Zx3 = (*mpND_Zx3);
            prevA3 = (*mpND_a3);

            //Motor equations
            dpe = d / (pi * 2);
            //dpe = dpr * eps;       //For variable displacement motor
            ble = B + Zc1 * dpe*dpe + Zc2 * dpe*dpe + Zx3;
            gamma = 1 / (cl * (Zc1 + Zc2) + 1);
            c1a = (cl * Zc2 + 1) * gamma * c1 + cl * gamma * Zc1 * c2;
            c2a = (cl * Zc1 + 1) * gamma * c2 + cl * gamma * Zc2 * c1;
            ct = c1a * dpe - c2a * dpe - c3;
            mIntegrator.setDamping(ble / J * mTimestep);
            mIntegrator.integrateWithUndo(ct/J);
            w3 = mIntegrator.valueFirst();
            a3 = mIntegrator.valueSecond();

            //Ideal Flow
            q1a = -dpe * w3;
            q2a = -q1a;
            p1 = c1a + gamma * Zc1 * q1a;
            p2 = c2a + gamma * Zc2 * q2a;

            //Cavitation Check
            bool cav=false;
            if (p1 < 0.0) {
                c1 = 0.0;
                Zc1 = 0;
                cav = true;
            }
            if (p2 < 0.0) {
                c2 = 0.0;
                Zc2 = 0;
                cav = true;
            }
            if(cav) {
                ble = B + Zc1 * dpe*dpe + Zc2 * dpe*dpe + Zx3;
                gamma = 1 / (cl * (Zc1 + Zc2) + 1);
                c1a = (cl * Zc2 + 1) * gamma * c1 + cl * gamma * Zc1 * c2;
                c2a = (cl * Zc1 + 1) * gamma * c2 + cl * gamma * Zc2 * c1;
                ct = c1a * dpe - c2a * dpe - c3;
                mIntegrator.setDamping(ble / J * mTimestep);
                mIntegrator.redoIntegrate(ct/J);
                w3 = mIntegrator.valueFirst();

                q1a = -dpe * w3;
                p1 = c1a + gamma * Zc1 * q1a;
                p2 = c2a + gamma * Zc2 * q2a;

                if(p1<=0) {
                    p1 = 0;
                    q1a = std::max(q1a, 0.0);
                    w3 = std::min(w3, 0.0);
                }
                if(p2<=0) {
                    p2 = 0;
                    q1a = std::min(q1a, 0.0);
                    w3 = std::max(w3, 0.0);
                }
                if(w3 > 0.0) {
                    a3 = mIntegrator.valueSecond();     //Only change a3 if w3 was not limited due to cavitation
                }
                else {
                    a3 = prevA3;
                }
                mIntegrator.initializeValues(ct/J, a3, w3);
                q2a = -q1a;
            }

            //Leakage Flow
            q1leak = -cl * (p1 - p2);
            q2leak = -q1leak;

            //Effective Flow
            q1 = q1a + q1leak;
            q2 = q2a + q2leak;

            //Torque
            t3 = c3 + w3 * Zx3;

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_t3) = t3;
            (*mpND_a3) = a3;
            (*mpND_w3) = w3;
        }
    };
}

#endif // HYDRAULICFIXEDDISPLACEMENTMOTORQ_H
