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
//! @file   SignalStepExponentialDelay.hpppp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-08-01
//!
//! @brief Contains a Step With Exponential Delay signal generator
//!
//$Id$

#ifndef SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED
#define SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStepExponentialDelay : public ComponentSignal
    {

    private:
        double *mpBaseValue;
        double *mpAmplitude;
        double *mpStepTime;
        double *mpTimeConstant;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStepExponentialDelay();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("tao", "Time Constant of Delay", "", 1.0, &mpTimeConstant);
            addInputVariable("t_step", "Step Time", "Time", 1.0, &mpStepTime);

            addOutputVariable("out", "", "", &mpOut);
        }


        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            // StepExponentialDelay Equations
            if (mTime < (*mpStepTime))
            {
                (*mpOut) = (*mpBaseValue);     //Before Step
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude) * exp(-(mTime-(*mpStepTime))/(*mpTimeConstant));     //After Step
            }
        }
    };
}

#endif // SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED
