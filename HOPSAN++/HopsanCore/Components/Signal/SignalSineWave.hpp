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
//              Offset →                                //
//                                                      //
//                   XXX         XXX        ↑           //
//                  X   X       X   X       | Amplitude //
// Zero →  XXXXX   X     X     X     X      ↓           //
//                X       X   X       X                 //
//              XX         XXX         XXX              //
//                                                      //
//              ↑           ←1/Frequency→               //
//          StartTime                                   //
//////////////////////////////////////////////////////////

#ifndef SIGNALSINEWAVE_HPP_INCLUDED
#define SIGNALSINEWAVE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSineWave : public ComponentSignal
    {

    private:
        double mStartTime;
        double mFrequency;
        double mAmplitude;
        double mOffset;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSineWave("SineWave");
        }

        SignalSineWave(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSineWave";
            mStartTime = 0.0;
            mFrequency = 1.0;
            mAmplitude = 1.0;
            mOffset = 0.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("StartTime", "Start Time", "[s]", mStartTime);
            registerParameter("Frequency", "Frequencty", "[Hz]", mFrequency);
            registerParameter("Amplitude", "Amplitude", "[-]", mAmplitude);
            registerParameter("Offset", "Offset", "[s]", mOffset);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Sinewave Equations
            if (mTime < mStartTime)
            {
                (*mpND_out) = 0.0;     //Before start
            }
            else
            {
                (*mpND_out) = mAmplitude*sin((mTime-mStartTime)*mFrequency*2*3.14159265 - mOffset);
            }
        }
    };
}

#endif // SIGNALSINEWAVE_HPP_INCLUDED
