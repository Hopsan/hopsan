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
//! @file   SignalSecondOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Second Order Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALSECONORDERFILTER_HPP_INCLUDED
#define SIGNALSECONORDERFILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSecondOrderFilter : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double mWnum, mDnum, mWden, mDden, mK;
        double mMin, mMax;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderFilter();
        }

        void configure()
        {

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mK = 1.0;
            mWnum = 1.0e10;
            mDnum = 1.0;
            mWden = 1000;
            mDden = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("k", "Gain", "[-]", mK, Constant);
            registerParameter("omega_1", "Numerator break frequency", "[rad/s]", mWnum, Constant);
            registerParameter("delta_1", "Numerator damp coefficient", "[-]", mDnum, Constant);
            registerParameter("omega_2", "Denominator break frequency", "[rad/s]", mWden, Constant);
            registerParameter("delta_2", "Denominator damp coefficient", "[-]", mDden, Constant);
            registerParameter("y_min", "Lower output limit", "[-]", mMin, Constant);
            registerParameter("y_max", "Upper output limit", "[-]", mMax, Constant);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::Value, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            double num[3];
            double den[3];

            num[2] = mK/(mWnum*mWnum);
            num[1] = mK*2.0*mDnum/mWnum;
            num[0] = mK;
            den[2] = 1.0/pow(mWden, 2);
            den[1] = 2.0*mDden/mWden;
            den[0] = 1.0;

            mTF2.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in), mMin, mMax);

            //Writes out the value for time "zero"
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF2.update(*mpND_in);
        }
    };
}

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED
