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
