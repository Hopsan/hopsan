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
        double wnum, wden, k;
        double min, max;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderFilter("Filter");
        }

        SignalFirstOrderFilter(const std::string name) : ComponentSignal(name)
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

            num[0] = k/wnum;
            num[1] = k;
            den[0] = 1.0/wden;
            den[1] = 1.0;

            mFilter.initialize(mTimestep, num, den, (*mpND_in), (*mpND_in), min, max);

            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mFilter.update((*mpND_in));
        }
    };
}

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


