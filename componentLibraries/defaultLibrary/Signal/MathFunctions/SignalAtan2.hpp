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
//! @file   SignalAtan2.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-04-12
//!
//! @brief Contains a signal atan2 function component
//!

#ifndef SIGNALATAN2_HPP_INCLUDED
#define SIGNALATAN2_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAtan2 : public ComponentSignal
    {

    private:
        double *mpND_inY, *mpND_inX, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalAtan2();
        }

        void configure()
        {
            addInputVariable("inY", "", "", 0.0, &mpND_inY);
            addInputVariable("inX", "", "", 0.0, &mpND_inX);
            addOutputVariable("out", "atan2(in)","",&mpND_out);
         }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) =atan2(*mpND_inY,*mpND_inX);
        }
    };
}

#endif // SIGNALATAN2_HPP_INCLUDED
