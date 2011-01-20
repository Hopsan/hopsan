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
        double *mpND_in1, *mpND_in2, *mpND_out;
        bool inputBool1, inputBool2;
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
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
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
            //Xor operator equation
            inputBool1 = doubleToBool(*mpND_in1);
            inputBool2 = doubleToBool(*mpND_in2);
            (*mpND_out) = boolToDouble( (inputBool1 || inputBool2) && !(inputBool1 && inputBool2) );
        }
    };
}
#endif // SIGNALXOR_HPP_INCLUDED
