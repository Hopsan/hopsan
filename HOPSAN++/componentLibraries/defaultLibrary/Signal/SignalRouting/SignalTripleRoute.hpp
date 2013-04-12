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
//! @file   SignalTripleRoute.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2011-08-29
//!
//! @brief Contains a signal routering component with two inputs
//!
//$Id$

#ifndef SIGNALTRIPLEROUTE_HPP_INCLUDED
#define SIGNALTRIPLEROUTE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalTripleRoute : public ComponentSignal
    {

    private:
        double *mpND_route, *mpND_in1, *mpND_in2, *mpND_in3, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalTripleRoute();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addInputVariable("in3", "", "", 0, &mpND_in3);
            addInputVariable("route", "Input selection", "", 0, &mpND_route);
            addOutputVariable("out", "Selected input", "", &mpND_out);
        }


        void initialize()
        {

        }


        void simulateOneTimestep()
        {
            if((*mpND_route) < 1.5 )
            {
                (*mpND_out) = (*mpND_in1);
            }
            else if(*mpND_route < 2.5)
            {
                (*mpND_out) = (*mpND_in2);
            }
            else
            {
                (*mpND_out) = (*mpND_in3);
            }
        }
    };
}
#endif
