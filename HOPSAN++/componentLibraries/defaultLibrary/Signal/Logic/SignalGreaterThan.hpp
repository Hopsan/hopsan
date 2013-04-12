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
//! @file   SignalGreaterThan.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-18
//!
//! @brief Contains a Greater Than Component
//!
//$Id$

#ifndef SIGNALGREATERTHAN_HPP_INCLUDED
#define SIGNALGREATERTHAN_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGreaterThan : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out, *mpLimit;

    public:
        static Component *Creator()
        {
            return new SignalGreaterThan();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addInputVariable("x_limit", "Limit Value", "", 0.0, &mpLimit);
            addOutputVariable("out", "in>x_limit", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Greater than equations
            (*mpND_out) = boolToDouble( (*mpND_in) > (*mpLimit) );
        }
    };
}

#endif // SIGNALGREATERTHAN_HPP_INCLUDED
