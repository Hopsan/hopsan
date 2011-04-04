//!
//! @file   SignalDivide.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical division function
//!
//$Id$

#ifndef SIGNALDIVIDE_HPP_INCLUDED
#define SIGNALDIVIDE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDivide : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDivide("Divide");
        }

        SignalDivide(const std::string name) : ComponentSignal(name)
        {

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::VALUE, 0);
            mpND_in2 = getSafeNodeDataPtr(mpIn2, NodeSignal::VALUE, 1);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, 0);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_in1) / (*mpND_in2);
        }
    };
}

#endif // SIGNALDIVIDE_HPP_INCLUDED
