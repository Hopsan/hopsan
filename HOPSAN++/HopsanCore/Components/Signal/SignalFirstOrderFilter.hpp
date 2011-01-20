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

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalFirstOrderFilter : public ComponentSignal
    {

    private:
        FirstOrderFilter mFilter;
        double mWnum, mWden, mK;
        double mMin, mMax;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderFilter("Filter");
        }

        SignalFirstOrderFilter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalFirstOrderFilter";

            mMin = -1.5E+300;
            mMax = 1.5E+300;
            mWnum = 1E+10;
            mWden = 1000.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("k", "Gain", "-", mK);
            registerParameter("wnum", "Numerator break frequency", "rad/s", mWnum);
            registerParameter("wden", "Denominator break frequency", "rad/s", mWden);
            registerParameter("min", "Output Lower limit", "-", mMin);
            registerParameter("max", "Output Upper limit", "-", mMax);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[2];
            double den[2];

            num[0] = mK/mWnum;
            num[1] = mK;
            den[0] = 1.0/mWden;
            den[1] = 1.0;

            mFilter.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in), mMin, mMax);

            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mFilter.update((*mpND_in));
        }
    };
}

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


