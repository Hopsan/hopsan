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
//$Id: SignalAnimationSlider.hpp 6778 2014-03-21 16:44:11Z petno25 $

#ifndef SIGNALANIMATIONLAMP_HPP_INCLUDED
#define SIGNALANIMATIONLAMP_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAnimationLamp : public ComponentSignal
    {

    private:
        double *mpIn;

    public:
        static Component *Creator()
        {
            return new SignalAnimationLamp();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
        }


        void initialize()
        {

        }


        void simulateOneTimestep()
        {

        }
    };
}

#endif //SIGNALANIMATIONLAMP_HPP_INCLUDED
