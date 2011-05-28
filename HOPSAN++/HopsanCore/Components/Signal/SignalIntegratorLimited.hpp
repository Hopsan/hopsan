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
//! @file   SignalIntegratorLimited.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-17
//!
//! @brief Contains a Signal Integrator Component with Limited Output
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegratorLimited : public ComponentSignal
    {

    private:
        double mMin, mMax;
        double mPrevU, mPrevY;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited("IntegratorLimited");
        }

        SignalIntegratorLimited(const std::string name) : ComponentSignal(name)
        {

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("y_min", "Lower output limit", "[-]", mMin);
            registerParameter("y_max", "Upper output limit", "[-]", mMax);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            limitValue(startY, mMin, mMax);
            mPrevU = startY;
            mPrevU = startY;
        }


        void simulateOneTimestep()
        {
            //Filter equations
            //Bilinear transform is used
            (*mpND_out) = mPrevY + mTimestep/2.0*((*mpND_in) + mPrevU);

            limitValue((*mpND_out), mMin, mMax);

            //Update filter:
            mPrevU = (*mpND_in);
            mPrevY = (*mpND_out);
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


