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
//! @file   SignalSoftStep.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-15
//!
//! @brief Contains a soft step generator
//!
//$Id$

///////////////////////////////////////////////
//                    StopTime               //
//                       â                   //
//                                           //
//                       XXXXXX  â           //
//                      X        |           //
//                     X         | Amplitude //
//                     X         |           //
//                    X          |           //
// BaseValue â  XXXXXX           â           //
//                                           //
//                   â                       //
//               StartTime                   //
///////////////////////////////////////////////

#ifndef SIGNALSOFTSTEP_HPP_INCLUDED
#define SIGNALSOFTSTEP_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSoftStep : public ComponentSignal
    {

    private:
        double *mpStartTime;
        double *mpStopTime;
        double *mpBaseValue;
        double *mpAmplitude;
        double *mpOffset;
        double *mpOut;
        Port *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalSoftStep();
        }

        void configure()
        {
            addInputVariable("t_start", "Start Time", "Time", 1.0, &mpStartTime);
            addInputVariable("t_end", "Stop Time", "Time", 2.0, &mpStopTime);
            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);

            addOutputVariable("out","","", &mpOut);
        }

        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            //Sinewave Equations
            const double startT = (*mpStartTime);
            const double stopT = (*mpStopTime);
            const double frequency = pi/(stopT-startT); //omega = 2pi/T, T = (stoptime-starttime)*4

            if (mTime < startT)
            {
                (*mpOut) = (*mpBaseValue);     //Before start
            }
            else if (mTime >= startT && mTime < stopT)
            {
                (*mpOut) = (*mpBaseValue) + 0.5*(*mpAmplitude)*sin((mTime-startT)*frequency - pi/2.0) + (*mpAmplitude)*0.5;
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);
            }
        }
    };
}

#endif // SIGNALSOFTSTEP_HPP_INCLUDED
