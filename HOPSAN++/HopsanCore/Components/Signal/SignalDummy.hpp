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
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id$

#ifndef SIGNALDUMMY_HPP_INCLUDED
#define SIGNALDUMMY_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDummy : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;

        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDummy("Dummy");
        }

        SignalDummy(const std::string name) : ComponentSignal(name)
        {

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = 1;
            for(size_t i=0; i<(*mpND_in); ++i)
            {
                (*mpND_out) = (*mpND_out) * i;
            }
        }
    };
}

#endif // SIGNALDUMMY_HPP_INCLUDED
