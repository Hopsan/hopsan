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
//! @file   SignalFirstOrderTransferFunction.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-08-03
//!
//! @brief Contains a Signal First Order transfer function
//!
//$Id$

#ifndef SIGNALFIRSTORDERTRANSFERFUNCTION_HPP_INCLUDED
#define SIGNALFIRSTORDERTRANSFERFUNCTION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalFirstOrderTransferFunction : public ComponentSignal
    {

    private:
        FirstOrderTransferFunction mTF;
        double mNum[2], mDen[2];

        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderTransferFunction();
        }

        SignalFirstOrderTransferFunction() : ComponentSignal()
        {
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            mNum[0] = 1;
            mNum[1] = 1;
            mDen[0] = 1;
            mDen[1] = 1;

            registerParameter("a_1", "S^1 numerator coefficient", "[-]", mNum[1]);
            registerParameter("a_0", "S^0 numerator coefficient", "[-]", mNum[0]);

            registerParameter("b_1", "S^1 denominator coefficient", "[-]", mDen[1]);
            registerParameter("b_0", "S^0 denominator coefficient", "[-]", mDen[0]);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            mTF.initialize(mTimestep, mNum, mDen, getStartValue(mpOut, NodeSignal::VALUE), getStartValue(mpOut, NodeSignal::VALUE));

            //Writes out the value for time "zero"
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF.update(*mpND_in);
        }
    };
}

#endif
