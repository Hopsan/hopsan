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
//! @file   HydraulicPressureSourceC.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEC_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSourceC : public ComponentC
    {
    private:
        Port *mpP1;
        double *mpP, *mpP1_p, *mpP1_c, *mpP1_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSourceC();
        }

        void configure()
        {
            addInputVariable("p", "Set pressure", "Pa", 1.0e5, &mpP);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            disableStartValue(mpP1, NodeHydraulic::Pressure);
            disableStartValue(mpP1, NodeHydraulic::WaveVariable);
            disableStartValue(mpP1, NodeHydraulic::CharImpedance);
            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            (*mpP1_p) = (*mpP);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            *mpP1_c = *mpP;
            *mpP1_Zc = 0.0;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
