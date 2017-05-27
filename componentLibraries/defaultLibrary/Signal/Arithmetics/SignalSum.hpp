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
//! @file   SignalSum.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-04-04
//!
//! @brief Contains a mathematical multiport summator component
//!
//$Id$

#ifndef SIGNALSUM_HPP_INCLUDED
#define SIGNALSUM_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>
#include <sstream>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSum : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_out;
        Port *mpMultiInPort;

    public:
        static Component *Creator()
        {
            return new SignalSum();
        }

        void configure()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", "", Port::NotRequired);
            addOutputVariable("out", "sum of inputs", "", &mpND_out);
        }


        void initialize()
        {
            // We need at least one dummy port even if no port is connected
            nInputs = std::max(mpMultiInPort->getNumPorts(),size_t(1));

            mNDp_in_vec.resize(nInputs);
            for (size_t i=0; i<nInputs; ++i)
            {
                mNDp_in_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInPort, i, NodeSignal::Value);
            }
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            double sum = 0;
            for (size_t i=0; i<nInputs; ++i)
            {
                sum += *mNDp_in_vec[i];
            }
            (*mpND_out) = sum; //Write value to output node
        }
    };
}
#endif
