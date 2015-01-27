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
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-05-07
//!
//! @brief Contains a Signal Adjustable Slider (does nothing in the simulation)
//!
//$Id$

#ifndef SIGNALANIMATIONSWITCH_HPP_INCLUDED
#define SIGNALANIMATIONSWITCH_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAnimationSwitch : public ComponentSignal
    {

    private:

    public:
        static Component *Creator()
        {
            return new SignalAnimationSwitch();
        }

        void configure()
        {
            addOutputVariable("out", "", "");
        }


        void initialize()
        {

        }


        void simulateOneTimestep()
        {

        }
    };
}

#endif //SIGNALANIMATIONSWITCH_HPP_INCLUDED
