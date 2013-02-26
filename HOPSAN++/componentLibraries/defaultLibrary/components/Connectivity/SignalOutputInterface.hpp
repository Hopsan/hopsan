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
//! @file   SignalOutputInterface.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-24
//!
//! @brief Contains a signal interface component for communication with other software
//!
//$Id$

#ifndef SIGNALOUTPUTINTERFACE_HPP_INCLUDED
#define SIGNALOUTPUTINTERFACE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalOutputInterface : public ComponentSignal
    {

    private:
        Port *mpOut;
  //      double *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalOutputInterface();
        }

        void configure()
        {
            mpOut = addReadPort("in", "NodeSignal");
        }


        void initialize()
        {
  //          mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            //Interfacing is handled through readnode/writenode from the RT wrapper file
        }
    };
}

#endif // SIGNALOUTPUTINTERFACE_HPP_INCLUDED
