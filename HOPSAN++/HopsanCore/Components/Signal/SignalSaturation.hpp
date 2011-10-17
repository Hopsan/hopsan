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
//! @file   SignalSaturation.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a signal saturation component
//!
//$Id$

#ifndef SIGNALSATURATION_HPP_INCLUDED
#define SIGNALSATURATION_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSaturation : public ComponentSignal
    {

    private:
        double mUpperLimit;
        double mLowerLimit;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSaturation("Saturation");
        }

        SignalSaturation(const std::string name) : ComponentSignal(name)
        {
            mUpperLimit = 1.0;
            mLowerLimit = -1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("y_upper", "Upper Limit", "[-]", mUpperLimit);
            registerParameter("y_lower", "Lower Limit", "[-]", mLowerLimit);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
               //Gain equations
            if ( (*mpND_in) > mUpperLimit )
            {
                (*mpND_out) = mUpperLimit;
            }
            else if ( (*mpND_in) < mLowerLimit )
            {
                (*mpND_out) = mLowerLimit;
            }
            else
            {
                (*mpND_out) = (*mpND_in);
            }
        }
    };
}

#endif // SIGNALSATURATION_HPP_INCLUDED
