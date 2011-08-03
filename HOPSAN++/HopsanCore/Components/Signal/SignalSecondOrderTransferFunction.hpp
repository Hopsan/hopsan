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
//! @file   SignalSecondOrderTransferFunction.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-04-27
//!
//! @brief Contains a Signal Second Order transfer function
//!
//$Id$

#ifndef SIGNALSECONORDERTRANSFERFUNCTION_HPP_INCLUDED
#define SIGNALSECONORDERTRANSFERFUNCTION_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSecondOrderTransferFunction : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mFilter;
        double a0, a1, a2, b0, b1, b2;

        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderTransferFunction();
        }

        SignalSecondOrderTransferFunction() : ComponentSignal()
        {

            a0 = 1;
            a1 = 1;
            a2 = 1;
            b0 = 1;
            b1 = 1;
            b2 = 1;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("a_0", "First numerator coefficient", "[-]", a0);
            registerParameter("a_1", "Second numerator coefficient", "[-]", a1);
            registerParameter("a_2", "Third numerator coefficient", "[-]", a2);
            registerParameter("b_0", "First denominator coefficient", "[-]", b0);
            registerParameter("b_1", "Second denominator coefficient", "[-]", b1);
            registerParameter("b_2", "Third denominator coefficient", "[-]", b2);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[3];
            double den[3];

            num[0] = a0;
            num[1] = a1;
            num[2] = a2;
            den[0] = b0;
            den[1] = b1;
            den[2] = b2;

            mFilter.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in));

            //Writes out the value for time "zero"
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mFilter.update(*mpND_in);
        }
    };
}

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED
