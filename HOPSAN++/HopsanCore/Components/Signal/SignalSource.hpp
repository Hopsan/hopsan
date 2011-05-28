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
//! @file   SignalSource.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Source Component
//!
//$Id$

#ifndef SIGNALSOURCE_HPP_INCLUDED
#define SIGNALSOURCE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSource : public ComponentSignal
    {

    private:
        double mValue;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSource("Source");
        }

        SignalSource(const std::string name) : ComponentSignal(name)
        {
            mValue = 1.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("y", "Source Value", "[-]", mValue);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, mValue);

            //Initialize value to the node
           (*mpND_out) = mValue;
        }


        void simulateOneTimestep()
        {
           (*mpND_out) = mValue;          //Temporary RT solution  

			//Nothing to do (only one write port can exist in the node, so no one else shall write to the value)
        }
    };
}

#endif // SIGNALSOURCE_HPP_INCLUDED
