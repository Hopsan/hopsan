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
//! @file   SignalStaircase.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-16
//!
//! @brief Contains a signal staircase function source
//!
//$Id$

#ifndef SIGNALSTAIRCASE_HPP_INCLUDED
#define SIGNALSTAIRCASE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalStaircase : public ComponentSignal
{
    private:
        double *mpStartT, *mpStepHeight, *mpStepWidth, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStaircase();
        }

        void configure()
        {
            // Register changable parameters to the HOPSAN++ core
            addInputVariable("T_start", "Start Time", "s", 0.0, &mpStartT);
            addInputVariable("H_step", "Step Height", "-", 1.0, &mpStepHeight);
            addInputVariable("W_step", "Step Width", "s", 1.0, &mpStepWidth);

            // Add ports to the component, (the defaulvalue will be the base level and is changable as parameter)
            addOutputVariable("out", "Stair case output", "", 0.0, &mpOut);
        }


        void initialize()
        {
            // No startvale calculation, value from startvalue in outpor will be used
        }

        void simulateOneTimestep()
        {
            // +0.5*min(mtimestep,stepWidth) to avoid double!=int nummeric accuracy issue
            (*mpOut) = (*mpStepHeight)*floor(std::max(0.0, mTime-(*mpStartT)+0.5*std::min(mTimestep,(*mpStepWidth)))/(*mpStepWidth));
        }
    };
}

#endif
