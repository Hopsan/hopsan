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
//! @file   SignalSink.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Sink Component
//!
//$Id$

#ifndef SIGNALSINK_HPP_INCLUDED
#define SIGNALSINK_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSink : public ComponentSignal
    {

    private:
        Port *mpIn;

    public:
        static Component *Creator()
        {
            return new SignalSink("Sink");
        }

        SignalSink(const std::string name) : ComponentSignal(name)
        {

            mpIn = addReadMultiPort("in", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            //Nothing to initilize
        }


        void simulateOneTimestep()
        {
            //Nothing to do
        }

        void finalize()
        {
            //Nothing to do
        }
    };
}

#endif // SIGNALSINK_HPP_INCLUDED
