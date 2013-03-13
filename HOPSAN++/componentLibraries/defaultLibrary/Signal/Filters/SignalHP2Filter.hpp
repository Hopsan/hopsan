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
//! @file   SignalLP2Filter.hpp
//! @author Robert Braun <karl.pettersson@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a Signal Second Order High Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALHP2FILTER_HPP_INCLUDED
#define SIGNALHP2FILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHP2Filter : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double mW, mD, mMin, mMax;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHP2Filter();
        }

        void configure()
        {

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1000.0;
            mD=1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("omega", "Break frequency", "[rad/s]", mW, Constant);
            registerParameter("delta", "Damp coefficient", "[-]", mD, Constant);
            registerParameter("y_min", "Lower output limit", "[-]", mMin, Constant);
            registerParameter("y_max", "Upper output limit", "[-]", mMax, Constant);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[3];
            double den[3];

            num[2] = 1.0/(mW*mW);
            num[1] = 0.0;
            num[0] = 0.0;
            den[2] = 1.0/(mW*mW);
            den[1] = 2.0*mD/mW;
            den[0] = 1.0;

            mTF2.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in), mMin, mMax);

            //Writes out the value for time "zero"
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF2.update((*mpND_in));
        }
    };
}

#endif // SIGNALHP2FILTER_HPP_INCLUDED


