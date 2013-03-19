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
//! @file   SignalMultiply.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical multiplication function
//!
//$Id$

#ifndef SIGNALMULTIPLY_HPP_INCLUDED
#define SIGNALMULTIPLY_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMultiply : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalMultiply();
        }

        void configure()
        {

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            //If only one input port is conncted, the other shall be 1 (= multiply by 1).
            //If no input ports are connected, mpND_output shall be 0, so one of the inputs are set to 0.
            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::Value, 1);
            mpND_in2 = getSafeNodeDataPtr(mpIn2, NodeSignal::Value, 1);

            if(!mpIn1->isConnected() && !mpIn2->isConnected())
            {
                (*mpND_in1 = 0);
            }

            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
                //Multiplication equation
            (*mpND_out) = (*mpND_in1) * (*mpND_in2);
        }
    };
}

#endif // SIGNALMULTIPLY_HPP_INCLUDED
