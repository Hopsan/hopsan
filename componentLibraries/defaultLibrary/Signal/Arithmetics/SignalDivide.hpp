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
//! @file   SignalDivide.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical division function
//!
//$Id$

#ifndef SIGNALDIVIDE_HPP_INCLUDED
#define SIGNALDIVIDE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDivide : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDivide();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addOutputVariable("out", "in1/in2", "", &mpND_out);
        }


        void initialize()
        {
            // We do a weaker check for division be zero at first time step, to avoid initial value troubles.
            // Simulation is allowed to continue and output value is set to zero.
            // User gets a warning message.
            if(*mpND_in2 == 0)
            {
                addWarningMessage("Division by zero at first time step. Output value set to zero.");
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in1) / (*mpND_in2);
            }
        }

        void simulateOneTimestep()
        {
            // Stop simulation if division by zero.
            if(*mpND_in2 == 0)
            {
                addErrorMessage("Division by zero.");
                stopSimulation();
            }
            (*mpND_out) = (*mpND_in1) / (*mpND_in2);
        }
    };
}

#endif // SIGNALDIVIDE_HPP_INCLUDED
