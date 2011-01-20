//!
//! @file   SignalAnd.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-19
//!
//! @brief Contains a logical or operator
//!
//$Id$

#ifndef SIGNALAND_HPP_INCLUDED
#define SIGNALAND_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAnd : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalAnd("And");
        }

        SignalAnd(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalAnd";

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::VALUE);
            mpND_in2 = getSafeNodeDataPtr(mpIn2, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //And operator equation
            (*mpND_out) = boolToDouble(doubleToBool((*mpND_in1)) && doubleToBool((*mpND_in2)));
        }
    };
}
#endif // SIGNALAND_HPP_INCLUDED
