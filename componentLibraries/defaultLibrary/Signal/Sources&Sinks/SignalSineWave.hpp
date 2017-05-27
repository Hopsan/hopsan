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
//! @file   SignalSineWave.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a sine wave signal generator
//!
//$Id$

//////////////////////////////////////////////////////////
//                                                      //
//              Offset â                                //
//                                                      //
//                   XXX         XXX        â           //
//                  X   X       X   X       | Amplitude //
// Zero â  XXXXX   X     X     X     X      â           //
//                X       X   X       X                 //
//              XX         XXX         XXX              //
//                                                      //
//              â           â1/Frequencyâ               //
//          StartTime                                   //
//////////////////////////////////////////////////////////

#ifndef SIGNALSINEWAVE_HPP_INCLUDED
#define SIGNALSINEWAVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities/AuxiliarySimulationFunctions.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSineWave : public ComponentSignal
    {

    private:
        double *mpStartTime;
        double *mpFrequency;
        double *mpAmplitude;
        double *mpPhaseTOffset;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSineWave();
        }

        void configure()
        {
            addInputVariable("f", "Frequencty", "Hz", 1.0, &mpFrequency);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("y_offset", "(Phase) Offset", "Time", 0.0, &mpPhaseTOffset);
            addInputVariable("t_start", "Start Time", "Time", 0.0, &mpStartTime);

            addOutputVariable("out", "Sinus wave output", "", &mpOut);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Sinewave Equations
            if (mTime < (*mpStartTime))
            {
                (*mpOut) = 0.0;     //Before start
            }
            else
            {
                // out = A * sin( (T-Tstart-Toffset)*2*pi*f )
                (*mpOut) = (*mpAmplitude) * sin( (mTime-(*mpStartTime)-(*mpPhaseTOffset)) * 2.0*pi*(*mpFrequency) );
            }
        }
    };
}

#endif // SIGNALSINEWAVE_HPP_INCLUDED
