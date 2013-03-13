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
//! @file   SignalSign.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2012-02-29
//!
//! @brief Contains a signal sign function component
//!
//$Id$

#ifndef SIGNALSIGN_HPP_INCLUDED
#define SIGNALSIGN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSign : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalSign();
        }

        void configure()
        {
            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = sign(*mpND_in);
        }
    };
}

#endif // SIGNALSIGN_HPP_INCLUDED
