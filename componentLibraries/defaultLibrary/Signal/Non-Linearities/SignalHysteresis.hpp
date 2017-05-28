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
//! @file   SignalHysteresis.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-04
//!
//! @brief Contains a Signal Hysteresis Component
//!
//$Id$

#ifndef SIGNALHYSTERESIS_HPP_INCLUDED
#define SIGNALHYSTERESIS_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHysteresis : public ComponentSignal
    {

    private:
        double *mpHysteresisWidth;
        Delay mDelayedInput;
        ValveHysteresis mHyst;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalHysteresis();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addInputVariable("y_h", "Width of the Hysteresis", "", 1.0, &mpHysteresisWidth);

            addOutputVariable("out", "", "", &mpND_out);
        }


        void initialize()
        {
            mDelayedInput.initialize(1, (*mpND_in));
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            //Hysteresis equations
            (*mpND_out) = mHyst.getValue((*mpND_in), (*mpHysteresisWidth), mDelayedInput.getOldest());
            mDelayedInput.update((*mpND_out));
        }
    };
}

#endif // SIGNALHYSTERESIS_HPP_INCLUDED
