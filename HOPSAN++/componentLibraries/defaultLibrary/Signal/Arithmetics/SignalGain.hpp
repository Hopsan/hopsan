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
//! @file   SignalGain.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id$

#ifndef SIGNALGAIN_HPP_INCLUDED
#define SIGNALGAIN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <algorithm>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGain : public ComponentSignal
    {

    private:
        double mGain;
        Port *mpIn, *mpOut;

        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalGain();
        }

        void configure()
        {
            mGain = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("k", "Gain value", "[-]", mGain);

            disableStartValue(mpOut,NodeSignal::Value);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::Value, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            // Now make sure the output initial value is based on the input
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mGain * (*mpND_in);
        }
    };
}

#endif // SIGNALGAIN_HPP_INCLUDED
