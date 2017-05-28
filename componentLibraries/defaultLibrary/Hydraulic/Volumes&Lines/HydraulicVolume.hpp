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
//! @file   HydraulicVolume.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains a Hydraulic Volume Component
//!
//$Id$

#ifndef HYDRAULICVOLUME_HPP_INCLUDED
#define HYDRAULICVOLUME_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolume : public ComponentC
    {

    private:
        double mZc;
        double mV;
        double mBetae;
        double mPh;

        Port *mpP1, *mpP2;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc;
        double *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        double *mpAlpha;

    public:
        static Component *Creator()
        {
            return new HydraulicVolume();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addInputVariable("alpha", "Low pass coefficient to dampen standing delayline waves", "-", 0.1 ,&mpAlpha);

            addConstant("V", "Volume", "m^3", 1.0e-3, mV);
            addConstant("Beta_e", "Bulkmodulus", "Pa", 1.0e9, mBetae);
            addConstant("P_high", "High pressure (for animation)", "Pa", 2e7, mPh);

            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
            setDefaultStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
            setDefaultStartValue(mpP2, NodeHydraulic::Flow, 0.0);
            setDefaultStartValue(mpP2, NodeHydraulic::Pressure, 1.0e5);

            disableStartValue(mpP1, NodeHydraulic::WaveVariable);
            disableStartValue(mpP2, NodeHydraulic::WaveVariable);
            disableStartValue(mpP1, NodeHydraulic::CharImpedance);
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

            double alpha = (*mpAlpha);
            mZc = mBetae/mV*mTimestep/(1.0-alpha); //Need to be updated at simulation start since it is volume and bulk that are set.

            //Write to nodes
            (*mpP1_c) = getDefaultStartValue(mpP2,NodeHydraulic::Pressure)+mZc*getDefaultStartValue(mpP2,NodeHydraulic::Flow);
            (*mpP1_Zc) = mZc;
            (*mpP2_c) = getDefaultStartValue(mpP1,NodeHydraulic::Pressure)+mZc*getDefaultStartValue(mpP1,NodeHydraulic::Flow);
            (*mpP2_Zc) = mZc;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q1, c1, q2, c2, c10, c20, alpha;

            //Get variable values from nodes
            q1 = (*mpP1_q);
            q2 = (*mpP2_q);
            c1 = (*mpP1_c);
            c2 = (*mpP2_c);
            alpha = (*mpAlpha);

            //Volume equations
            c10 = c2 + 2.0*mZc * q2;     //These two equations are from old Hopsan
            c20 = c1 + 2.0*mZc * q1;

            c1 = alpha*c1 + (1.0-alpha)*c10;
            c2 = alpha*c2 + (1.0-alpha)*c20;

            //Write new values to nodes
            (*mpP1_c) = c1;
            (*mpP1_Zc) = mZc;
            (*mpP2_c) = c2;
            (*mpP2_Zc) = mZc;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICVOLUME_HPP_INCLUDED
