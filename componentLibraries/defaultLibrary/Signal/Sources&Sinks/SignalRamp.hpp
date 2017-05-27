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
//! @file   SignalRamp.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a ramp signal generator
//!
//$Id$

///////////////////////////////////////////////////
//                       StopTime                //
//                          â                    //
//                                               //
//                          XXXXXXX  â           //
//                        XX         |           //
//                      XX           | Amplitude //
//                    XX             |           //
// BaseValue â  XXXXXX               â           //
//                                               //
//                   â                           //
//               StartTime                       //
///////////////////////////////////////////////////

#ifndef SIGNALRAMP_HPP_INCLUDED
#define SIGNALRAMP_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalRamp : public ComponentSignal
    {

    private:
        double *mpBaseValue;
        double *mpAmplitude;
        double *mpStartTime;
        double *mpStopTime;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalRamp();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("t_start", "Start Time", "Time", 1.0, &mpStartTime);
            addInputVariable("t_end", "Stop Time", "Time", 2.0, &mpStopTime);

            addOutputVariable("out", "Ramp output", "", &mpOut);
        }


        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            const double startT = (*mpStartTime);
            const double stopT = (*mpStopTime);

            // Step Equations
            if (mTime < startT)
            {
                (*mpOut) = (*mpBaseValue);     //Before ramp
            }
            else if (mTime >= startT && mTime < stopT)
            {
                (*mpOut) = ((mTime - startT) / (stopT - startT)) * (*mpAmplitude) + (*mpBaseValue);     //During ramp
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);     //After ramp
            }
        }
    };
}

#endif // SIGNALRAMP_HPP_INCLUDED
