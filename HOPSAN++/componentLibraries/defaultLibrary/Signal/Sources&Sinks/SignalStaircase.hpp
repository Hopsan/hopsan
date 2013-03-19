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

#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class SignalStaircase : public ComponentC
{

    private:
        double startT, stepHeight, stepWidth;

        //Node data pointers
        double *mpND_out;

        //Ports
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStaircase();
        }

        void configure()
        {
            //Set member attributes
            startT=0;
            stepHeight=1;
            stepWidth=1;

            //Add ports to the component
            mpOut = addWritePort("out", "NodeSignal");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("T_start", "Start Time", "[s]", startT);
            registerParameter("H_step", "Step Height", "[-]", stepHeight);
            registerParameter("W_step", "Step Width", "[-]", stepWidth);
        }


        void initialize()
        {

        }

        void simulateOneTimestep()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value, 0.0);

            double out=0;

            if(mTime > startT+stepWidth)
            {
                out += stepHeight;
            }

            out += std::max(0.0, stepHeight*floor((mTime-startT-stepWidth)/stepWidth));

            (*mpND_out) = out;
        }
    };
}

#endif
