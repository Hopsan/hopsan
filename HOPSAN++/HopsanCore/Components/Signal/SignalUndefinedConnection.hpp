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
//! @file   SignalUndefinedConnection.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-05
//!
//! @brief Contains a Signal Undefined Connection Component (for Real-Time targets)
//!
//$Id$

#ifndef SIGNALUNDEFINEDCONNECTION_HPP_INCLUDED
#define SIGNALUNDEFINEDCONNECTION_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalUndefinedConnection : public ComponentSignal
    {

    private:
        Port *mpOut;
        double X;
        double *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalUndefinedConnection("UndefinedConnection");
        }

        SignalUndefinedConnection(const std::string name) : ComponentSignal(name)
        {
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("X",  "Value", "[-]",  X);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_out) = X;
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = X;
        }
    };
}

#endif // SIGNALUNDEFINEDCONNECTION_HPP_INCLUDED
