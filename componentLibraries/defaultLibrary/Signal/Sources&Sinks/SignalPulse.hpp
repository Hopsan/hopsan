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
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-14
//!
//! @brief Contains a pulse signal generator
//!
//$Id$

////////////////////////////////////////////////
//                    XXXXX       â           //
//                    X   X       | Amplitude //
//                    X   X       |           //
// BaseValue â  XXXXXXX   XXXXXXX â           //
//                    â   â                   //
//            StartTime   StopTime            //
////////////////////////////////////////////////

#ifndef SIGNALPULSE_HPP_INCLUDED
#define SIGNALPULSE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPulse : public ComponentSignal
    {

    private:
        double *mpOut, *mpY0, *mpTstart, *mpTend, *mpYa;

    public:
        static Component *Creator()
        {
            return new SignalPulse();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpY0);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpYa);
            addInputVariable("t_start", "Start Time", "Time", 1.0, &mpTstart);
            addInputVariable("t_end", "Stop Time", "Time", 2.0, &mpTend);

            addOutputVariable("out", "Pulse", "", &mpOut);
        }


        void initialize()
        {
            // Write initial value
            (*mpOut) = (*mpY0);
        }


        void simulateOneTimestep()
        {
                //Step Equations
            const double time = mTime+0.5*mTimestep;
            if ( time >= (*mpTstart) && time < (*mpTend))
            {
                (*mpOut) = (*mpY0) + (*mpYa);     //During pulse
            }
            else
            {
                (*mpOut) = (*mpY0);               //Not during pulse
            }
        }
    };
}

#endif // SIGNALPULSE_HPP_INCLUDED
