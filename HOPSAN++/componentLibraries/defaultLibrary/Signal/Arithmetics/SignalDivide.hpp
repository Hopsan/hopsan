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
//! @file   SignalDivide.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical division function
//!
//$Id$

#ifndef SIGNALDIVIDE_HPP_INCLUDED
#define SIGNALDIVIDE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDivide : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDivide();
        }

        void configure()
        {

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NotRequired);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::VALUE, 0);
            mpND_in2 = getSafeNodeDataPtr(mpIn2, NodeSignal::VALUE, 1);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, 0);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_in1) / (*mpND_in2);
        }
    };
}

#endif // SIGNALDIVIDE_HPP_INCLUDED
