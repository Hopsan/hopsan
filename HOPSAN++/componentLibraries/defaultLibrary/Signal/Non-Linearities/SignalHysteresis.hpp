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
//! @file   SignalHysteresis.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-04
//!
//! @brief Contains a Signal Hysteresis Component
//!
//$Id$

#ifndef SIGNALHYSTERESIS_HPP_INCLUDED
#define SIGNALHYSTERESIS_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHysteresis : public ComponentSignal
    {

    private:
        double mHysteresisWidth;
        Delay mDelayedInput;
        ValveHysteresis mHyst;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHysteresis();
        }

        void configure()
        {
            mHysteresisWidth = 1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("y_h", "Width of the Hysteresis", "[-]", mHysteresisWidth);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            mDelayedInput.initialize(1, 0.0);

            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            //Hysteresis equations
            (*mpND_out) = mHyst.getValue((*mpND_in), mHysteresisWidth, mDelayedInput.getOldest());
            mDelayedInput.update((*mpND_out));
        }
    };
}

#endif // SIGNALHYSTERESIS_HPP_INCLUDED
