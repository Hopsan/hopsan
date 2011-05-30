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

            registerParameter("y_0", "Base Value", "[-]", mBaseValue);
            registerParameter("t_start", "Start Time", "[-]", mStartTime);
            registerParameter("t_end", "Stop Time", "[-]", mStopTime);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);

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
