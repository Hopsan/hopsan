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
//! @file   SignalConstant.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Constant Component
//!
//$Id$

#ifndef SIGNALCONSTANT_HPP_INCLUDED
#define SIGNALCONSTANT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalConstant : public ComponentSignal
    {

    private:
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalConstant();
        }

        void configure()
        {
            addOutputVariable("y", "Constant value", "-", 1.0, &mpOut);
        }


        void initialize()
        {
            // Nothing to do
        }

        void simulateOneTimestep()
        {
            //Nothing to do (only one write port can exist in the node, so no one else shall write to the value)
        }
    };
}

#endif // SIGNALCONSTANT_HPP_INCLUDED
