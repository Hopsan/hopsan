/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, BjÃ¶rn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at LinkÃ¶ping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SignalInputInterface.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-24
//!
//! @brief Contains a signal interface component for communication with other software
//!
//$Id$

#ifndef SIGNALINPUTINTERFACE_HPP_INCLUDED
#define SIGNALINPUTINTERFACE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalInputInterface : public ComponentSignal
    {

    public:
        static Component *Creator()
        {
            return new SignalInputInterface();
        }

        void configure()
        {
            addOutputVariable("out", "", "");
        }

        void initialize() {}
        void simulateOneTimestep() {}
    };
}

#endif // SIGNALINPUTINTERFACE_HPP_INCLUDED
