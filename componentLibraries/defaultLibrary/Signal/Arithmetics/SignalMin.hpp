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
//! @file   SignalMin.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-21
//!
//! @brief Contains a maximum component
//!
//$Id$

#ifndef SIGNALMIN_HPP_INCLUDED
#define SIGNALMIN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMin : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_out;
        Port *mpMultiInPort;

    public:
        static Component *Creator()
        {
            return new SignalMin();
        }

        void configure()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", "", Port::NotRequired);
            addOutputVariable("out", "Min of the inputs", "", &mpND_out);
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
            double min = (*mNDp_in_vec[0]);
            for (size_t i=1; i<nInputs; ++i)
            {
                if(*mNDp_in_vec[i]<min)
                {
                    min = *mNDp_in_vec[i];
                }
            }
            (*mpND_out) = min; //Write value to output node
        }
    };
}
#endif
