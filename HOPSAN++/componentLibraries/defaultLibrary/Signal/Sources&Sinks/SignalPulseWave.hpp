/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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
            addInputVariable("y_0", "Base Value", "-", 0.0);
            addInputVariable("t_start", "Start Time", "s", 0.0);
            addInputVariable("dT", "Time Period", "s", 1.0);
            addInputVariable("D", "Duty Cycle, (ratio 0<=x<=1)", "-", 0.5);
            addInputVariable("y_A", "Amplitude", "-", 1.0);

            addOutputVariable("out", "PulseWave", "");
        }


        void initialize()
        {
            mpBaseValue = getSafeNodeDataPtr("y_0", NodeSignal::Value);
            mpStartTime = getSafeNodeDataPtr("t_start", NodeSignal::Value);
            mpPeriodT = getSafeNodeDataPtr("dT", NodeSignal::Value);
            mpDutyCycle = getSafeNodeDataPtr("D", NodeSignal::Value);
            mpAmplitude = getSafeNodeDataPtr("y_A", NodeSignal::Value);
            mpOut = getSafeNodeDataPtr("out", NodeSignal::Value);

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
