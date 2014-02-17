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

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalQuadRoute : public ComponentSignal
    {

    private:
        double *mpND_route, *mpND_in1, *mpND_in2, *mpND_in3, *mpND_in4, *mpND_out;
        double limit12, limit23, limit34;

    public:
        static Component *Creator()
        {
            return new SignalQuadRoute();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addInputVariable("in3", "", "", 0, &mpND_in3);
            addInputVariable("in4", "", "", 0, &mpND_in4);
            addInputVariable("route", "Input selection", "", 0, &mpND_route);
            addOutputVariable("out", "Selected input", "", &mpND_out);

            addConstant("limit12", "Limit value between input 1 and 2", "-", 0.5, limit12);
            addConstant("limit23", "Limit value between input 2 and 3", "-", 1.5, limit23);
            addConstant("limit34", "Limit value between input 3 and 4", "-", 2.5, limit34);
        }


        void initialize()
        {

        }


        void simulateOneTimestep()
        {
            if((*mpND_route) < limit12 )
            {
                (*mpND_out) = (*mpND_in1);
            }
            else if((*mpND_route) < limit23 )
            {
                (*mpND_out) = (*mpND_in2);
            }
            else if((*mpND_route) < limit34 )
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
