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
//$Id: SignalAnimationSlider.hpp 3542 2011-10-25 08:06:49Z petno25 $

#ifndef SIGNALANIMATIONSLIDER_HPP_INCLUDED
#define SIGNALANIMATIONSLIDER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAnimationSlider : public ComponentSignal
    {

    private:
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalAnimationSlider();
        }

        void configure()
        {
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {

        }


        void simulateOneTimestep()
        {

        }
    };
}

#endif //SIGNALANIMATIONSLIDER_HPP_INCLUDED
