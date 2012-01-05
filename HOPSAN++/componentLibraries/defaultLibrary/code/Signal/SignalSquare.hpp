/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköing University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SignalSquare.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2010-09-28
//!
//! @brief Contains a mathematical square component
//!
//$Id$

#ifndef SIGNALSQUARE_HPP_INCLUDED
#define SIGNALSQUARE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSquare : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSquare();
        }

        SignalSquare() : ComponentSignal()
        {

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
//            simulateOneTimestep();
            mpOut->writeNode(NodeSignal::VALUE, 0.0);
        }


        void simulateOneTimestep()
        {

            //Get variable values from nodes
            double in;

            if (mpIn->isConnected())       //In-port connected
            {
                in = mpIn->readNode(NodeSignal::VALUE);

            }
            else
            {
                in = 0;                                           //Nothing connected
            }

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, in*in);
        }
    };
}
#endif // SIGNALSQUARE_HPP_INCLUDED
