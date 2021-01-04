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
//! @file   HydraulicTurbulentOrifice.hpp
//! @author Karl Pettersson
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Turbulent Orifice
//! @ingroup HydraulicComponents
//!
//$Id$

#ifndef HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
#define HYDRAULICTURBULENTORIFICE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic orifice with turbulent flow of Q-Type. Uses TurbulentFlowFunction to calculate the flow.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTurbulentOrifice : public ComponentQ
    {
    private:
        TurbulentFlowFunction qTurb;

        Port *mpP1, *mpP2;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        double *mpA, *mpCq, *mpRho;

    public:
        static Component *Creator()
        {
            return new HydraulicTurbulentOrifice();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addInputVariable("A", "Area", "Area", 0.00001, &mpA);
            addInputVariable("C_q", "Flow coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
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
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            double p1,q1, p2, q2, c1, Zc1, c2, Zc2;
            double A, Cq, rho, Kc;

            //Get variable values from nodes
            c1 = (*mpP1_c);
            Zc1 = (*mpP1_Zc);
            c2 = (*mpP2_c);
            Zc2 = (*mpP2_Zc);
            A = fabs(*mpA);
            Cq = (*mpCq);
            rho = (*mpRho);

            //Orifice equations
            Kc = Cq*A*sqrt(2.0/rho);
            qTurb.setFlowCoefficient(Kc);
            q2 = qTurb.getFlow(c1,c2,Zc1,Zc2);
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            bool cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = qTurb.getFlow(c1,c2,Zc1,Zc2);
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
        }
    };
}

#endif // HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
