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
//! @file   DummyComponent.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-10-25
//!
//! @brief Contains an empty dummy component
//!
//$Id$

#ifndef DUMMYCOMPONENT_HPP_INCLUDED
#define DUMMYCOMPONENT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class DummyComponent : public ComponentSignal
    {
    private:

    public:
        static Component *Creator()
        {
            return new DummyComponent();
        }

        void configure()
        {
            // Do nothing
        }


        void initialize()
        {
            // Do nothing
        }


        void simulateOneTimestep()
        {
            // Do nothing
        }
    };
}

#endif
