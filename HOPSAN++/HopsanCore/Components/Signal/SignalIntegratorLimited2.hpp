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
//! @file   SignalIntegrator2.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Integrator Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED2_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED2_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegratorLimited2 : public ComponentSignal
    {

    private:
        IntegratorLimited mIntegrator;
        double mStartY;
        double mMin, mMax;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited2("Integrator");
        }

        SignalIntegratorLimited2(const std::string name) : ComponentSignal(name)
        {
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            registerParameter("y_min", "Lower output limit", "[-]", mMin);
            registerParameter("y_max", "Upper output limit", "[-]", mMax);

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            mIntegrator.initialize(mTimestep, (*mpND_in), mStartY, mMin, mMax);

            (*mpND_out) = (*mpND_in);
            limitValue((*mpND_out), mMin, mMax);
        }


        void simulateOneTimestep()
        {

            //Filter equation
            (*mpND_out) = mIntegrator.update((*mpND_in));
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED2_HPP_INCLUDED


