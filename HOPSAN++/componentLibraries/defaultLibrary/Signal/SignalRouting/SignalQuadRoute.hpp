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
//! @file   SignalQuadRoute.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2011-08-29
//!
//! @brief Contains a signal routering component with two inputs
//!
//$Id$

#ifndef SIGNALQUADROUTE_HPP_INCLUDED
#define SIGNALQUADROUTE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalQuadRoute : public ComponentSignal
    {

    private:
        double *mpND_route, *mpND_in1, *mpND_in2, *mpND_in3, *mpND_in4, *mpND_out;
        Port *mpRoute, *mpIn1, *mpIn2, *mpIn3, *mpIn4, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalQuadRoute();
        }

        void configure()
        {
            mpRoute = addReadPort("route", "NodeSignal", Port::Required);
            mpIn1 = addReadPort("in1", "NodeSignal", Port::NotRequired);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NotRequired);
            mpIn3 = addReadPort("in3", "NodeSignal", Port::NotRequired);
            mpIn4 = addReadPort("in4", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            mpND_route  = getSafeNodeDataPtr(mpRoute,  NodeSignal::Value, 0);
            mpND_in1  = getSafeNodeDataPtr(mpIn1,  NodeSignal::Value, 0);
            mpND_in2  = getSafeNodeDataPtr(mpIn2,  NodeSignal::Value, 0);
            mpND_in3  = getSafeNodeDataPtr(mpIn3,  NodeSignal::Value, 0);
            mpND_in4  = getSafeNodeDataPtr(mpIn4,  NodeSignal::Value, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value, 0);
        }


        void simulateOneTimestep()
        {
            if((*mpND_route) < 1.5 )
            {
                (*mpND_out) = (*mpND_in1);
            }
            else if((*mpND_route) < 2.5 )
            {
                (*mpND_out) = (*mpND_in2);
            }
            else if((*mpND_route) < 3.5 )
            {
                (*mpND_out) = (*mpND_in3);
            }
            else
            {
                (*mpND_out) = (*mpND_in4);
            }
        }
    };
}
#endif
