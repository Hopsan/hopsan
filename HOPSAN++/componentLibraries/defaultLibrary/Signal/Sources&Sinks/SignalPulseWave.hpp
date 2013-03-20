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
        double mBaseValue;
        double mStartTime;
        double mPeriodT;
        double mDutyCycle;
        double mAmplitude;
        double *mpND_out;
        Port *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalPulseWave();
        }

        void configure()
        {
            mBaseValue = 0.0;
            mStartTime = 0.0;
            mPeriodT = 1.0;
            mAmplitude = 1.0;
            mDutyCycle = 0.5;

            mpOutPort = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("y_0", "Base Value", "[-]", mBaseValue);
            registerParameter("t_start", "Start Time", "[s]", mStartTime);
            registerParameter("dT", "Time Period", "[s]", mPeriodT);
            registerParameter("D", "Duty Cycle, (ratio 0<=x<=1)", "[-]", mDutyCycle);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);

            disableStartValue(mpOutPort, NodeSignal::Value);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::Value);

            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
            const double time = (mTime-mStartTime);
            bool high = (time - std::floor(time/mPeriodT)*mPeriodT) < mDutyCycle*mPeriodT;

            if ( (mTime >= mStartTime) && high)
            {
                (*mpND_out) = mBaseValue + mAmplitude;     //During pulse
            }
            else
            {
                (*mpND_out) = mBaseValue;                  //Not during pulse
            }
        }
    };
}

#endif // SIGNALPULSEWAVE_HPP
