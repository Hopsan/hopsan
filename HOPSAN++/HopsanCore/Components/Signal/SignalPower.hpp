//!
//! @file   SignalPower.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2010-09-28
//!
//! @brief Contains a mathematical square component
//!
//$Id$

#ifndef SIGNALPOWER_HPP_INCLUDED
#define SIGNALPOWER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPower : public ComponentSignal
    {

    private:
        double x;
        Port *mpIn, *mpOut;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalPower("Power");
        }

        SignalPower(const std::string name) : ComponentSignal(name)
        {
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            x=0;

            registerParameter("x", "Exponential", "-", x);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = pow((*mpND_in), x);
        }
    };
}
#endif // SIGNALPOWER_HPP_INCLUDED
