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
        double *mpUpperLimit;
        double *mpLowerLimit;
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSaturation();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0, &mpIn);
            addInputVariable("y_upper", "Upper Limit", "", 1.0, &mpUpperLimit);
            addInputVariable("y_lower", "Lower Limit", "", -1.0, &mpLowerLimit);
            addOutputVariable("out", "", "", &mpOut);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Saturate equations
            if ( (*mpIn) > (*mpUpperLimit) )
            {
                (*mpOut) = (*mpUpperLimit);
            }
            else if ( (*mpIn) < (*mpLowerLimit) )
            {
                (*mpOut) = (*mpLowerLimit);
            }
            else
            {
                (*mpOut) = (*mpIn);
            }
        }
    };
}

#endif // SIGNALSATURATION_HPP_INCLUDED
