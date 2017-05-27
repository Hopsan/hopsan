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
//! @file   SignalPulse.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-03-29
//!
//! @brief Contains a pulse wave (train) signal generator
//!
//$Id$

#ifndef SIGNALPULSEWAVE_HPP
#define SIGNALPULSEWAVE_HPP

#include "ComponentEssentials.h"
#include <cmath>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPulseWave : public ComponentSignal
    {

    private:
        double *mpBaseValue;
        double *mpStartTime;
        double *mpPeriodT;
        double *mpDutyCycle;
        double *mpAmplitude;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalPulseWave();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("t_start", "Start Time", "Time", 0.0, &mpStartTime);
            addInputVariable("dT", "Time Period", "Time", 1.0, &mpPeriodT);
            addInputVariable("D", "Duty Cycle, (ratio 0<=x<=1)", "", 0.5, &mpDutyCycle);

            addOutputVariable("out", "PulseWave", "", &mpOut);
        }


        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            // +0.5*mTimestep to avoid ronding issues
            const double time = (mTime-(*mpStartTime)+0.5*mTimestep);
            const double periodT = (*mpPeriodT);
            const bool high = (time - std::floor(time/periodT)*periodT) < (*mpDutyCycle)*periodT;

            if ( (time > 0) && high)
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);     //During pulse
            }
            else
            {
                (*mpOut) = (*mpBaseValue);                  //Not during pulse
            }
        }
    };
}

#endif // SIGNALPULSEWAVE_HPP
