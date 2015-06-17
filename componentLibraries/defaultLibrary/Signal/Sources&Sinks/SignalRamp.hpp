/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
