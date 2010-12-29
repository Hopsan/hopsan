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
        double *in1, *in2, *output;
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
            in1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
            in2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            output = mpOut->getNodeDataPtr(NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //And operator equation
            (*output) = boolToDouble(doubleToBool((*in1)) && doubleToBool((*in2)));
        }
    };
}
#endif // SIGNALAND_HPP_INCLUDED
