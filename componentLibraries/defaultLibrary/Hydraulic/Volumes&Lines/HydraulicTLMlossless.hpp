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
//! @file   HydraulicTLMlossless.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains a Hydraulic Lossless Transmission Line Component
//!
//$Id$

#ifndef HYDRAULICTLMLOSSLESS_HPP_INCLUDED
#define HYDRAULICTLMLOSSLESS_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTLMlossless : public ComponentC
    {

    private:
        double mTimeDelay;
        double *mpAlpha, *mpZc;

        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;

        Delay mDelayedC1;
        Delay mDelayedC2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicTLMlossless();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addInputVariable("alpha", "Low pass coefficient", "-", 0.0, &mpAlpha);
            addInputVariable("Z_c", "Impedance", "Pa s/m^3",  1.0e9, &mpZc);

            addConstant("deltat", "Time delay", "s",   0.1, mTimeDelay);

            disableStartValue(mpP1, NodeHydraulic::WaveVariable);
            disableStartValue(mpP1, NodeHydraulic::CharImpedance);
            disableStartValue(mpP2, NodeHydraulic::WaveVariable);
            disableStartValue(mpP2, NodeHydraulic::CharImpedance);
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

            double Zc;
            Zc = (*mpZc);

            //Write to nodes
            (*mpP1_q) = getDefaultStartValue(mpP1,NodeHydraulic::Flow);
            (*mpP1_p) = getDefaultStartValue(mpP1,NodeHydraulic::Pressure);
            (*mpP1_c) = getDefaultStartValue(mpP2,NodeHydraulic::Pressure)+Zc*getDefaultStartValue(mpP2,NodeHydraulic::Flow);
            (*mpP1_Zc) = Zc;
            (*mpP2_q) = getDefaultStartValue(mpP2,NodeHydraulic::Flow);
            (*mpP2_p) = getDefaultStartValue(mpP2,NodeHydraulic::Pressure);
            (*mpP2_c) = getDefaultStartValue(mpP1,NodeHydraulic::Pressure)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow);
            (*mpP2_Zc) = Zc;

            if (mTimeDelay-mTimestep < 0)
            {
                addWarningMessage("TimeDelay must be >= Ts");
                //stopSimulation();
            }

            // Initialize delay
            // We use -Ts to make the delay one step shorter as the TLM already have one built in time step delay
            //! @todo for Td=Ts the delay will actually be 2Ts (the delay and inherited) need if check to avoid using delays if Td=Ts
            mDelayedC1.initialize(mTimeDelay-mTimestep, mTimestep, (*mpP1_c));
            mDelayedC2.initialize(mTimeDelay-mTimestep, mTimestep, (*mpP2_c));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, p2, q2, c2, c10, c20, alpha, Zc;

            //Read variables from nodes
            p1 = (*mpP1_p);
            q1 = (*mpP1_q);
            p2 = (*mpP2_p);
            q2 = (*mpP2_q);
            c1 = (*mpP1_c);
            c2 = (*mpP2_c);
            alpha = (*mpAlpha);
            Zc = (*mpZc);

            //Delay Line equations
            c10 = p2 + Zc * q2;
            c20 = p1 + Zc * q1;
            c1  = alpha*c1 + (1.0-alpha)*c10;
            c2  = alpha*c2 + (1.0-alpha)*c20;

            //Write new values to nodes
            (*mpP1_c) = mDelayedC1.update(c1);
            (*mpP1_Zc) = Zc;
            (*mpP2_c) = mDelayedC2.update(c2);
            (*mpP2_Zc) = Zc;

        }
    };
}

#endif // HYDRAULICTLMLOSSLESS_HPP_INCLUDED
