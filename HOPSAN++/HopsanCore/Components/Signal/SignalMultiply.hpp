//!
//! @file   SignalMultiply.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical multiplication function
//!
//$Id$

#ifndef SIGNALMULTIPLY_HPP_INCLUDED
#define SIGNALMULTIPLY_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMultiply : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalMultiply("Multiply");
        }

        SignalMultiply(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalMultiply";

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            //If only one input port is conncted, the other shall be 1 (= multiply by 1).
            //If no input ports are connected, mpND_output shall be 0, so one of the inputs are set to 0.
            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::VALUE, 1);
            mpND_in2 = getSafeNodeDataPtr(mpIn2, NodeSignal::VALUE, 1);

            if(!mpIn1->isConnected() && !mpIn2->isConnected())
            {
                (*mpND_in1 = 0);
            }

            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
                //Multiplication equation
            (*mpND_out) = (*mpND_in1) * (*mpND_in2);
        }
    };
}

#endif // SIGNALMULTIPLY_HPP_INCLUDED
