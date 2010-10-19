//!
//! @file   SignalXor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-19
//!
//! @brief Contains a logical and operator
//!
//$Id$

#ifndef SIGNALXOR_HPP_INCLUDED
#define SIGNALXOR_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalXor : public ComponentSignal
    {

    private:
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalXor("Xor");
        }

        SignalXor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalXor";

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {

            //Get variable values from nodes
            double signal1 = mpIn1->readNode(NodeSignal::VALUE);
            double signal2 = mpIn2->readNode(NodeSignal::VALUE);

            //And operator equation
            double output = boolToDouble( (doubleToBool(signal1) || doubleToBool(signal2)) && !(doubleToBool(signal1) && doubleToBool(signal2)) );

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, output);
        }
    };
}
#endif // SIGNALXOR_HPP_INCLUDED
