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

//$Id$

#ifndef HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED
#define HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED

#include <iostream>
#include <cmath>
#include "ComponentEssentials.h"

#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVariableDisplacementPump : public ComponentQ
    {
    private:
        double *mpN, *mpDp, *mpKcp, *mpEps, *mpA;             // rad/s

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicVariableDisplacementPump();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addOutputVariable("a", "Angle", "", 0.0, &mpA);
            addInputVariable("eps", "Displacement setting", "", 1.0, &mpEps);
            addInputVariable("omega_p", "Angular velocity", "AngularVelocity", 50.0, &mpN);
            addInputVariable("D_p", "Displacement", "m^3/rev", 0.00005, &mpDp);
            addInputVariable("K_cp", "Leakage coefficient", "LeakageCoefficient", 1e-12, &mpKcp);
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
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double n, dp, Kcp, eps, p1, q1, c1, Zc1, p2, q2, c2, Zc2;
            bool cav;

            cav = false;
            n = (*mpN);
            dp = (*mpDp);
            Kcp = (*mpKcp);
            eps = (*mpEps);

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);

            //Variable Displacement Pump equations

            q2 = ( dp*n*eps/(2.0*pi) + Kcp*(c1-c2) ) / ( (Zc1+Zc2)*Kcp+1 );
            q2 = std::fmin(std::fmax(q2, -c2/Zc2), c1/Zc1); //Limit flow to indirectly limit pressures
            q1 = -q2;
            p2 = c2 + Zc2*q2;
            p1 = c1 + Zc1*q1;

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpA) += n*mTimestep;
        }
    };
}

#endif // HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED
