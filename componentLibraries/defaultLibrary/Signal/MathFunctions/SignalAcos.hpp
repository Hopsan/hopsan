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
//! @file   SignalAcos.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2011-10-18
//!
//! @brief Contains a signal acos function component
//!

#ifndef SIGNALACOS_HPP_INCLUDED
#define SIGNALACOS_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAcos : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out, *mpND_err, x;

    public:
        static Component *Creator()
        {
            return new SignalAcos();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "acos(in)","",&mpND_out);
            addOutputVariable("error", "error","",&mpND_err);
         }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            x=(*mpND_in);
            if(x>1.)
            {
            (*mpND_out) = acos(1.);
            (*mpND_err)=1.;
            }
            else if(x<-1.)
            {
            (*mpND_out) = acos(-1.);
            (*mpND_err)=1.;
            }
            else
            {
            (*mpND_out) = acos(*mpND_in);
            (*mpND_err)=0.;
            }
        }
    };
}

#endif // SIGNALACOS_HPP_INCLUDED
