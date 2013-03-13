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
        double mPhaseTOffset;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSineWave();
        }

        void configure()
        {
            mStartTime = 0.0;
            mFrequency = 1.0;
            mAmplitude = 1.0;
            mPhaseTOffset = 0.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("t_start", "Start Time", "[s]", mStartTime);
            registerParameter("f", "Frequencty", "[Hz]", mFrequency);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);
            registerParameter("y_offset", "(Phase) Offset", "[s]", mPhaseTOffset);

            disableStartValue(mpOut, NodeSignal::VALUE);
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
                (*mpND_out) = mAmplitude*sin(((mTime-mStartTime) - mPhaseTOffset)*2*M_PI*mFrequency);
            }
        }
    };
}

#endif // SIGNALSINEWAVE_HPP_INCLUDED
