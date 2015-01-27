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
//! @file   SignalNumericalInput.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-07-03
//!
//! @brief Contains a numerical input component (for animation)
//!
//$Id$

#ifndef SIGNALNUMERICALINPUT_HPP_INCLUDED
#define SIGNALNUMERICALINPUT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class SignalNumericalInput : public ComponentC
{

    private:
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalNumericalInput();
        }

        void configure()
        {
            //Add ports to the component
            addOutputVariable("out","","",0.0, &mpOut);
        }


        void initialize()
        {

        }

        void simulateOneTimestep()
        {

        }
    };
}

#endif
