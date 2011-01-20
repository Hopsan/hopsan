//!
//! @file   SignalSmallerThan.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-18
//!
//! @brief Contains a Smaller Than Component
//!
//$Id$

#ifndef SIGNALSMALLERTHAN_HPP_INCLUDED
#define SIGNALSMALLERTHAN_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSmallerThan : public ComponentSignal
    {

    private:
        double mLimit;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSmallerThan("SmallerThan");
        }

        SignalSmallerThan(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSmallerThan";
            mLimit = 0.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("x", "Limit Value", "-", mLimit);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Smaller than equations
            (*mpND_out) = boolToDouble( (*mpND_in) < mLimit );
        }
    };
}

#endif // SIGNALSMALLERTHAN_HPP_INCLUDED
