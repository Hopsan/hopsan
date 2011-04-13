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
//                       ↓                   //
//                                           //
//                       XXXXXX  ↑           //
//                      X        |           //
//                     X         | Amplitude //
//                     X         |           //
//                    X          |           //
// BaseValue →  XXXXXX           ↓           //
//                                           //
//                   ↑                       //
//               StartTime                   //
///////////////////////////////////////////////

#ifndef SIGNALSOFTSTEP_HPP_INCLUDED
#define SIGNALSOFTSTEP_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSoftStep : public ComponentSignal
    {

    private:
        double mStartTime;
        double mStopTime;
        double mBaseValue;
        double mAmplitude;
        double mFrequency;
        double mOffset;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSoftStep("SoftStep");
        }

        SignalSoftStep(const std::string name) : ComponentSignal(name)
        {
            mStartTime = 1.0;
            mStopTime = 2.0;
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mFrequency = pi/(mStopTime-mStartTime);       //omega = 2pi/T, T = (stoptime-starttime)*4

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("StartTime", "Start Time", "[s]", mStartTime);
            registerParameter("StopTime", "Stop Time", "[s]", mStopTime);
            registerParameter("BaseValue", "Base Value", "[-]", mBaseValue);
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
            //Sinewave Equations

            mFrequency = pi/(mStopTime-mStartTime);
            if (mTime < mStartTime)
            {
                (*mpND_out) = mBaseValue;     //Before start
            }
            else if (mTime > mStartTime && mTime < mStopTime)
            {
                (*mpND_out) = mBaseValue + 0.5*mAmplitude*sin((mTime-mStartTime)*mFrequency - 3.141592653589/2) + mAmplitude*0.5;
            }
            else
            {
                (*mpND_out) = mBaseValue + mAmplitude;
            }
        }
    };
}

#endif // SIGNALSOFTSTEP_HPP_INCLUDED
