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
//! @file   SignalDxSegare.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-06-03
//!
//! @brief Contains the derivative of the signal segment section area function component
//!

#ifndef SIGNALDXSEGARE_HPP_INCLUDED
#define SIGNALDXSEGARE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDxSegare : public ComponentSignal
    {

    private:
        double *mpND_x, *mpND_width, *mpND_diameter;

    public:
        static Component *Creator()
        {
            return new SignalDxSegare();
        }

        void configure()
        {
            addInputVariable("x", "", "", 0.0, &mpND_x);
            addInputVariable("diameter", "", "", 0.0, &mpND_diameter);
            addOutputVariable("width", "dxsegare(in)","",&mpND_width);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_width) = dxSegare(*mpND_x,*mpND_diameter);
        }
    };
}

#endif // SIGNALDXSEGARE_HPP_INCLUDED
