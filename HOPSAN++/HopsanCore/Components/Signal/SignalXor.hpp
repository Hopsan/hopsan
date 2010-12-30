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
        double *input1, *input2, *output;
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
            input1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
            input2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);

            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double();
            }

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Xor operator equation
            inputBool1 = doubleToBool(*input1);
            inputBool2 = doubleToBool(*input2);
            (*output) = boolToDouble( (inputBool1 || inputBool2) && !(inputBool1 && inputBool2) );
        }
    };
}
#endif // SIGNALXOR_HPP_INCLUDED
