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
//! @file   SignalFirstOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal First Order Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALFIRSTORDERFILTER_HPP_INCLUDED
#define SIGNALFIRSTORDERFILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalFirstOrderFilter : public ComponentSignal
    {

    private:
        FirstOrderTransferFunction mTF;
        double wnum, wden, k;
        double min, max;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderFilter();
        }

        void configure()
        {
            k = 1;
            min = -1.5E+300;
            max = 1.5E+300;
            wnum = 1E+10;
            wden = 1000.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("k", "Gain", "[-]", k);
            registerParameter("omega_num", "Numerator break frequency", "[rad/s]", wnum);
            registerParameter("omega_den", "Denominator break frequency", "[rad/s]", wden);
            registerParameter("y_min", "Lower output limit", "[-]", min);
            registerParameter("y_max", "Upper output limit", "[-]", max);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[2];
            double den[2];

            num[1] = k/wnum;
            num[0] = k;
            den[1] = 1.0/wden;
            den[0] = 1.0;

            mTF.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in), min, max);

            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF.update((*mpND_in));
        }
    };
}

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


