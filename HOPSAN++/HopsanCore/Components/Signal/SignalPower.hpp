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
//! @file   SignalPower.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2010-09-28
//!
//! @brief Contains a mathematical square component
//!
//$Id$

#ifndef SIGNALPOWER_HPP_INCLUDED
#define SIGNALPOWER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPower : public ComponentSignal
    {

    private:
        double x;
        Port *mpIn, *mpOut;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalPower("Power");
        }

        SignalPower(const std::string name) : ComponentSignal(name)
        {
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            x=0;

            registerParameter("x", "Exponential", "-", x);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = pow((*mpND_in), x);
        }
    };
}
#endif // SIGNALPOWER_HPP_INCLUDED
