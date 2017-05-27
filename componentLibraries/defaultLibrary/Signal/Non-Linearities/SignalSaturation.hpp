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
//! @file   SignalSaturation.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a signal saturation component
//!
//$Id$

#ifndef SIGNALSATURATION_HPP_INCLUDED
#define SIGNALSATURATION_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSaturation : public ComponentSignal
    {

    private:
        double *mpUpperLimit;
        double *mpLowerLimit;
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSaturation();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0, &mpIn);
            addInputVariable("y_upper", "Upper Limit", "", 1.0, &mpUpperLimit);
            addInputVariable("y_lower", "Lower Limit", "", -1.0, &mpLowerLimit);
            addOutputVariable("out", "", "", &mpOut);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Saturate equations
            if ( (*mpIn) > (*mpUpperLimit) )
            {
                (*mpOut) = (*mpUpperLimit);
            }
            else if ( (*mpIn) < (*mpLowerLimit) )
            {
                (*mpOut) = (*mpLowerLimit);
            }
            else
            {
                (*mpOut) = (*mpIn);
            }
        }
    };
}

#endif // SIGNALSATURATION_HPP_INCLUDED
