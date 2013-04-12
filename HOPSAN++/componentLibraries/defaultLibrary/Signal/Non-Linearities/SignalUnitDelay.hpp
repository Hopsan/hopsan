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
//! @file   SignalUnitDelay.hpppp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2013-01-11
//!
//! @brief Contains a Signal Time Delay Component
//!
//$Id$

#ifndef SIGNALUNITDELAY_HPP_INCLUDED
#define SIGNALUNITDELAY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalUnitDelay : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalUnitDelay();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) =  (*mpND_in);
        }
    };
}

#endif // SIGNALUNITDELAY_HPP_INCLUDED
