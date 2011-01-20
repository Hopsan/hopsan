//!
//! @file   SignalGreaterThan.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-18
//!
//! @brief Contains a Greater Than Component
//!
//$Id$

#ifndef SIGNALGREATERTHAN_HPP_INCLUDED
#define SIGNALGREATERTHAN_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGreaterThan : public ComponentSignal
    {

    private:
        double mLimit;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalGreaterThan("GreaterThan");
        }

        SignalGreaterThan(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalGreaterThan";
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
            //Greater than equations
            (*mpND_out) = boolToDouble( (*mpND_in) > mLimit );
        }
    };
}

#endif // SIGNALGREATERTHAN_HPP_INCLUDED
