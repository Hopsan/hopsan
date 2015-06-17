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
