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
//! @file   SignalDualRoute.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2011-08-29
//!
//! @brief Contains a signal routering component with two inputs
//!
//$Id$

#ifndef SIGNALDUALROUTE_HPP_INCLUDED
#define SIGNALDUALROUTE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDualRoute : public ComponentSignal
    {

    private:
        double *mpND_route;
        double *mpND_in1;
        double *mpND_in2;
        double *mpND_out;
        Port *mpRoute, *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDualRoute();
        }

        void configure()
        {
            mpRoute = addReadPort("route", "NodeSignal", Port::REQUIRED);
            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_route  = getSafeNodeDataPtr(mpRoute,  NodeSignal::VALUE, 0);
            mpND_in1  = getSafeNodeDataPtr(mpIn1,  NodeSignal::VALUE, 0);
            mpND_in2  = getSafeNodeDataPtr(mpIn2,  NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, 0);
        }


        void simulateOneTimestep()
        {
            if((*mpND_route) < 1.5 )
            {
                (*mpND_out) = (*mpND_in1);
            }
            else
            {
                (*mpND_out) = (*mpND_in2);
            }
        }
    };
}
#endif
