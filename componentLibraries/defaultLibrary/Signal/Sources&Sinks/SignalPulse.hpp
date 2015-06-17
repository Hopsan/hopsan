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
