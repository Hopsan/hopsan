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
//! @file   SignalSegare.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-03-07
//!
//! @brief Contains a signal segment section area function component
//!

#ifndef SIGNALSEGARE_HPP_INCLUDED
#define SIGNALSEGARE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSegare : public ComponentSignal
    {

    private:
        double *mpND_x, *mpND_area, *mpND_diameter;

    public:
        static Component *Creator()
        {
            return new SignalSegare();
        }

        void configure()
        {
            addInputVariable("x", "", "", 0.0, &mpND_x);
            addInputVariable("diameter", "", "", 0.0, &mpND_diameter);
            addOutputVariable("area", "segare(in)","",&mpND_area);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_area) = segare(*mpND_x,*mpND_diameter);
        }
    };
}

#endif // SIGNALSEGARE_HPP_INCLUDED
