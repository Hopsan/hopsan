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
//! @file   SignalStopSimulation.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-15
//!
//! @brief Contains a component for stopping a simulation
//!
//$Id$

#ifndef SIGNALSTOPSIMULATION_HPP_INCLUDED
#define SIGNALSTOPSIMULATION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStopSimulation : public ComponentSignal
    {

    private:
        double *mpND_in;
        Port *mpIn;

    public:
        static Component *Creator()
        {
            return new SignalStopSimulation();
        }

        void configure()
        {

            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, boolToDouble(false));
        }


        void simulateOneTimestep()
        {
            if(doubleToBool(*mpND_in))
            {
                this->stopSimulation();
            }
        }
    };
}
#endif // SIGNALSTOPSIMULATION_HPP_INCLUDED
