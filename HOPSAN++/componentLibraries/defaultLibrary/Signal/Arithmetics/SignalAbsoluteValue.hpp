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
//! @file   SignalAbsoluteValue.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2011-02-03
//!
//! @brief Contains a Signal Absolute Value Component
//!
//$Id$

#ifndef SIGNALABSOLUTEVALUE_HPP_INCLUDED
#define SIGNALABSOLUTEVALUE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAbsoluteValue : public ComponentSignal
    {

    private:
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalAbsoluteValue();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "ABS of in", "", &mpOut);
        }


        void initialize()
        {
            simulateOneTimestep();
        }

        void simulateOneTimestep()
        {
            if(*mpIn > 0)
            {
                (*mpOut) = (*mpIn);
            }
            else
            {
                (*mpOut) = -(*mpIn);
            }
        }
    };
}

#endif // SIGNALABSOLUTEVALUE_HPP_INCLUDED
