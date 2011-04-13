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
//                          ↓                    //
//                                               //
//                          XXXXXXX  ↑           //
//                        XX         |           //
//                      XX           | Amplitude //
//                    XX             |           //
// BaseValue →  XXXXXX               ↓           //
//                                               //
//                   ↑                           //
//               StartTime                       //
///////////////////////////////////////////////////

#ifndef SIGNALRAMP_HPP_INCLUDED
#define SIGNALRAMP_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalRamp : public ComponentSignal
    {

    private:
        double mBaseValue;
        double mAmplitude;
        double mStartTime;
        double mStopTime;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalRamp("Ramp");
        }

        SignalRamp(const std::string name) : ComponentSignal(name)
        {
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mStartTime = 1.0;
            mStopTime = 2.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("BaseValue", "Base Value", "[-]", mBaseValue);
            registerParameter("Amplitude", "Amplitude", "[-]", mAmplitude);
            registerParameter("StartTime", "Start Time", "[s]", mStartTime);
            registerParameter("StopTime", "Stop Time", "[s]", mStopTime);

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
            if (mTime < mStartTime)
            {
                (*mpND_out) = mBaseValue;     //Before ramp
            }
            else if (mTime >= mStartTime && mTime < mStopTime)
            {
                (*mpND_out) = ((mTime - mStartTime) / (mStopTime - mStartTime)) * mAmplitude + mBaseValue ;     //During ramp
            }
            else
            {
                (*mpND_out) = mBaseValue + mAmplitude;     //After ramp
            }
        }
    };
}

#endif // SIGNALRAMP_HPP_INCLUDED
