//!
//! @file   SignalAbsoluteValue.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2011-02-03
//!
//! @brief Contains a Signal Absolute Value Component
//!
//$Id$

#ifndef SIGNALABSOLUTEVALUE_HPP_INCLUDED
#define SIGNALABSOLUTEVALUE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAbsoluteValue : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalAbsoluteValue("AbsoluteValue");
        }

        SignalAbsoluteValue(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalAbsoluteValue";

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }

        void simulateOneTimestep()
        {
            if(*mpND_in > 0)
            {
                (*mpND_out) = (*mpND_in);
            }
            else
            {
                (*mpND_out) = -(*mpND_in);
            }
        }
    };
}

#endif // SIGNALABSOLUTEVALUE_HPP_INCLUDED
