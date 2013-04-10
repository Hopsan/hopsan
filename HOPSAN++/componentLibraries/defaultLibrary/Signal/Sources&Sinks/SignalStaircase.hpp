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
        double *mpStartT, *mpStepHeight, *mpStepWidth;

        //Node data pointers
        double *mpOut;

        //Ports
        Port *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalStaircase();
        }

        void configure()
        {
            // Register changable parameters to the HOPSAN++ core
            addInputVariable("T_start", "Start Time", "s", 0.0);
            addInputVariable("H_step", "Step Height", "-", 1.0);
            addInputVariable("W_step", "Step Width", "s", 1.0);

            // Add ports to the component, (the defaulvalue will be the base level and is changable as parameter)
            mpOutPort = addOutputVariable("out", "Stair case output", "", 0.0);
        }


        void initialize()
        {
            mpStartT = getSafeNodeDataPtr("T_start", NodeSignal::Value);
            mpStepHeight = getSafeNodeDataPtr("H_step", NodeSignal::Value);
            mpStepWidth = getSafeNodeDataPtr("W_step", NodeSignal::Value);

            mpOut = getSafeNodeDataPtr(mpOutPort, NodeSignal::Value);
        }

        void simulateOneTimestep()
        {
            // +0.5*min(mtimestep,stepWidth) to avoid double!=int nummeric accuracy issue
            (*mpOut) = (*mpStepHeight)*floor(std::max(0.0, mTime-(*mpStartT)+0.5*std::min(mTimestep,(*mpStepWidth)))/(*mpStepWidth));
        }
    };
}

#endif
