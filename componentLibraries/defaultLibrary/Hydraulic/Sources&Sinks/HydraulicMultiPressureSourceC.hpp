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
//! @file   HydraulicMultiPressureSourceC.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Multi Port Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICMULTIPRESSURESOURCEC_HPP_INCLUDED
#define HYDRAULICMULTIPRESSURESOURCEC_HPP_INCLUDED

#include <vector>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicMultiPressureSourceC : public ComponentC
    {
    private:
        Port *mpMP;
        size_t mNumPorts;
        std::vector<double*> mvpMP_p;
        std::vector<double*> mvpMP_q;
        std::vector<double*> mvpMP_c;
        std::vector<double*> mvpMP_Zc;
        double *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicMultiPressureSourceC();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("MP", "NodeHydraulic");
            addInputVariable("p", "Default pressure", "Pa", 1.0e5, &mpIn);

            disableStartValue(mpMP, NodeHydraulic::Pressure);
            disableStartValue(mpMP, NodeHydraulic::WaveVariable);
            disableStartValue(mpMP, NodeHydraulic::CharImpedance);
        }


        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();
            //! @todo write help function to set the size and contents of a these vectors automatically
            mvpMP_p.resize(mNumPorts);
            mvpMP_q.resize(mNumPorts);
            mvpMP_c.resize(mNumPorts);
            mvpMP_Zc.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpMP_p[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Pressure);
                mvpMP_q[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Flow);
                mvpMP_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::WaveVariable);
                mvpMP_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::CharImpedance);

                *(mvpMP_p[i]) = (*mpIn);    //Override the startvalue for the pressure
                *(mvpMP_q[i]) = getDefaultStartValue(mpMP, NodeHydraulic::Flow);
                *(mvpMP_c[i]) = (*mpIn);
                *(mvpMP_Zc[i]) = 0.0;
            }
        }


        void simulateOneTimestep()
        {
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *(mvpMP_c[i]) = (*mpIn);
                *(mvpMP_Zc[i]) = 0.0;
            }
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
