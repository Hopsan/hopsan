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
//! @file   SignalRamp.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a ramp signal generator
//!
//$Id$

///////////////////////////////////////////////////
//                       StopTime                //
//                          â                    //
//                                               //
//                          XXXXXXX  â           //
//                        XX         |           //
//                      XX           | Amplitude //
//                    XX             |           //
// BaseValue â  XXXXXX               â           //
//                                               //
//                   â                           //
//               StartTime                       //
///////////////////////////////////////////////////

#ifndef SIGNALRAMP_HPP_INCLUDED
#define SIGNALRAMP_HPP_INCLUDED

#include "ComponentEssentials.h"

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
            return new SignalRamp();
        }

        SignalRamp() : ComponentSignal()
        {
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mStartTime = 1.0;
            mStopTime = 2.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("y_0", "Base Value", "[-]", mBaseValue);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);
            registerParameter("t_start", "Start Time", "[s]", mStartTime);
            registerParameter("t_end", "Stop Time", "[s]", mStopTime);

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
