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
//! @file   SignalSmallerThan.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-18
//!
//! @brief Contains a Smaller Than Component
//!
//$Id$

#ifndef SIGNALSMALLERTHAN_HPP_INCLUDED
#define SIGNALSMALLERTHAN_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSmallerThan : public ComponentSignal
    {

    private:
        double mLimit;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSmallerThan("SmallerThan");
        }

        SignalSmallerThan(const std::string name) : ComponentSignal(name)
        {
            mLimit = 0.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("x_limit", "Limit Value", "-", mLimit);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Smaller than equations
            (*mpND_out) = boolToDouble( (*mpND_in) < mLimit );
        }
    };
}

#endif // SIGNALSMALLERTHAN_HPP_INCLUDED
