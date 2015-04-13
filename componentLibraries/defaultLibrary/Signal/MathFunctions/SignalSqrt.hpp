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
//! @file   SignalSqrt.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-04-12
//!
//! @brief Contains a signal asin function component
//!

#ifndef SIGNALSQRT_HPP_INCLUDED
#define SIGNALSQRT_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSqrt : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out, *mpND_err, x;

    public:
        static Component *Creator()
        {
            return new SignalSqrt();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "sqrt(in)","",&mpND_out);
            addOutputVariable("error", "error","",&mpND_err);
         }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            x=(*mpND_in);
            if(x<0.)
            {
            (*mpND_out) = 0.;
            (*mpND_err)=1.;
            }
             else
            {
            (*mpND_out) = sqrt(*mpND_in);
            (*mpND_err)=0.;
            }
        }
    };
}

#endif // SIGNALASQRT_HPP_INCLUDED
