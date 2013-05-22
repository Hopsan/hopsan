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

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSecondOrderTransferFunction : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double a0, a1, a2, b0, b1, b2;

        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderTransferFunction();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpND_in);
            addOutputVariable("out", "","",0.0, &mpND_out);

            addConstant("a_2", "S^2 numerator coefficient", "[-]", 1, a2);
            addConstant("a_1", "S^1 numerator coefficient", "[-]", 1, a1);
            addConstant("a_0", "S^0 numerator coefficient", "[-]", 1, a0);

            addConstant("b_2", "S^2 denominator coefficient", "[-]", 1, b2);
            addConstant("b_1", "S^1 denominator coefficient", "[-]", 1, b1);
            addConstant("b_0", "S^0 denominator coefficient", "[-]", 1, b0);
        }


        void initialize()
        {
            double num[3];
            double den[3];

            num[0] = a0;
            num[1] = a1;
            num[2] = a2;
            den[0] = b0;
            den[1] = b1;
            den[2] = b2;

            mTF2.initialize(mTimestep, num, den, *mpND_in, *mpND_out);

            // Do not write initial value to out port, its startvalue is used to initialize the filter
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF2.update(*mpND_in);
        }
    };
}

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED
