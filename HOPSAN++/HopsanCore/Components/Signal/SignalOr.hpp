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
//! @file   SignalOr.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-19
//!
//! @brief Contains a logical and operator
//!
//$Id$

#ifndef SIGNALOR_HPP_INCLUDED
#define SIGNALOR_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalOr : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalOr("Or");
        }

        SignalOr(const std::string name) : ComponentSignal(name)
        {

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::VALUE);
            mpND_in2 = getSafeNodeDataPtr(mpIn2, NodeSignal::VALUE);

            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Or operator equation
            (*mpND_out) = boolToDouble(doubleToBool(*mpND_in1) || doubleToBool(*mpND_in2));
        }
    };
}
#endif // SIGNALOR_HPP_INCLUDED
