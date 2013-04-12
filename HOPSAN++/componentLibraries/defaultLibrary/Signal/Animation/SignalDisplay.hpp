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
//! @file   SignalDisplay.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-11-15
//!
//! @brief Contains a signal display component
//!
//$Id$

#ifndef SIGNALDISPLAY_HPP_INCLUDED
#define SIGNALDISPLAY_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class SignalDisplay : public ComponentC
{

    private:

    public:
        static Component *Creator()
        {
            return new SignalDisplay();
        }

        void configure()
        {
            //Add ports to the component
            addInputVariable("in","","",0.0);
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
