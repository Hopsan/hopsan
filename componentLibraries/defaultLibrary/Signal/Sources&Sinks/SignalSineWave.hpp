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
