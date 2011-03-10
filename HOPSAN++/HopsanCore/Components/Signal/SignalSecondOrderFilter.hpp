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

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSecondOrderFilter : public ComponentSignal
    {

    private:
        SecondOrderFilter mFilter;
        double mWnum, mDnum, mWden, mDden, mK;
        double mMin, mMax;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderFilter("Filter");
        }

        SignalSecondOrderFilter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSecondOrderFilter";

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mK = 1.0;
            mWnum = 1.0e10;
            mDnum = 1.0;
            mWden = 1000;
            mDden = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("k", "Gain", "[-]", mK);
            registerParameter("wnum", "Numerator break frequency", "[rad/s]", mWnum);
            registerParameter("dnum", "Numerator damp coefficient", "[-]", mDnum);
            registerParameter("wden", "Denominator break frequency", "[rad/s]", mWden);
            registerParameter("dden", "Denominator damp coefficient", "[-]", mDden);
            registerParameter("min", "Output Lower limit", "[-]", mMin);
            registerParameter("max", "Output Upper limit", "[-]", mMax);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[3];
            double den[3];

            num[0] = mK/(mWnum*mWnum);
            num[1] = mK*2.0*mDnum/mWnum;
            num[2] = mK;
            den[0] = 1.0/pow(mWden, 2);
            den[1] = 2.0*mDden/mWden;
            den[2] = 1.0;

            mFilter.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in), mMin, mMax);

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
