//!
//! @file   SignalPulse.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-14
//!
//! @brief Contains a pulse signal generator
//!
//$Id$

////////////////////////////////////////////////
//                    XXXXX       ↑           //
//                    X   X       | Amplitude //
//                    X   X       |           //
// BaseValue →  XXXXXXX   XXXXXXX ↓           //
//                    ↑   ↑                   //
//            StartTime   StopTime            //
////////////////////////////////////////////////

#ifndef SIGNALPULSE_HPP_INCLUDED
#define SIGNALPULSE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPulse : public ComponentSignal
    {

    private:
        double mBaseValue;
        double mStartTime;
        double mStopTime;
        double mAmplitude;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalPulse("Pulse");
        }

        SignalPulse(const std::string name) : ComponentSignal(name)
        {
            mBaseValue = 0.0;
            mStartTime = 1.0;
            mStopTime = 2.0;
            mAmplitude = 1.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("BaseValue", "Base Value", "[-]", mBaseValue);
            registerParameter("StartTime", "Start Time", "[-]", mStartTime);
            registerParameter("StopTime", "Stop Time", "[-]", mStopTime);
            registerParameter("Amplitude", "Amplitude", "[-]", mAmplitude);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
                //Step Equations
            if (mTime > mStartTime && mTime < mStopTime)
            {
                (*mpND_out) = mBaseValue + mAmplitude;     //During pulse
            }
            else
            {
                (*mpND_out) = mBaseValue;                   //Not during pulse
            }
        }
    };
}

#endif // SIGNALPULSE_HPP_INCLUDED
