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
//! @file   SignalSub.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2012-02-29
//!
//! @brief Contains a mathematical multiport subtractor component
//!
//$Id$

#ifndef SIGNALSUB_HPP_INCLUDED
#define SIGNALSUB_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSub : public ComponentSignal
    {

    private:
        size_t nSumInputs, nSubInputs;
        std::vector<double *> mNDp_in_sum_vec, mNDp_in_sub_vec;
        double *mpND_out;
        Port *mpMultiInSumPort, *mpMultiInSubPort;

    public:
        static Component *Creator()
        {
            return new SignalSub();
        }

        void configure()
        {
            mpMultiInSumPort = addReadMultiPort("insum", "NodeSignal", "", Port::NotRequired);
            mpMultiInSubPort = addReadMultiPort("insub", "NodeSignal", "", Port::NotRequired);
            addOutputVariable("out", "sum(insum)-sum(insub)", "", &mpND_out);
        }


        void initialize()
        {
            //We need at least one dummy port even if no port is connected
            nSumInputs = std::max(mpMultiInSumPort->getNumPorts(), size_t(1));
            nSubInputs = std::max(mpMultiInSubPort->getNumPorts(), size_t(1));

            mNDp_in_sum_vec.resize(nSumInputs);
            for (size_t i=0; i<nSumInputs; ++i)
            {
                mNDp_in_sum_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInSumPort, i, NodeSignal::Value);
            }
            mNDp_in_sub_vec.resize(nSubInputs);
            for (size_t i=0; i<nSubInputs; ++i)
            {
                mNDp_in_sub_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInSubPort, i, NodeSignal::Value);
            }
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            double sum = 0;
            for (size_t i=0; i<nSumInputs; ++i)
            {
                sum += *mNDp_in_sum_vec[i];
            }
            for (size_t i=0; i<nSubInputs; ++i)
            {
                sum -= *mNDp_in_sub_vec[i];
            }
            (*mpND_out) = sum; //Write value to output node
        }
    };
}
#endif
