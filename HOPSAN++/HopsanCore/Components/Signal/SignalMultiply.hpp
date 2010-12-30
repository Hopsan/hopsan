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
        double *input1, *input2, *output;
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
            //If no input ports are connected, output shall be 0, so both inputs are set to 0.
            if(mpIn1->isConnected() && mpIn2->isConnected())
            {
                input1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
                input2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            }
            else if(mpIn1->isConnected() && !mpIn2->isConnected())
            {
                input1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
                input2 = new double(1);
            }
            else if(!mpIn1->isConnected() && mpIn2->isConnected())
            {
                input1 = new double(1);
                input2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input1 = new double(0);
                input2 = new double(0);
            }

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
                //Multiplication equation
            (*output) = (*input1) * (*input2);
        }
    };
}

#endif // SIGNALMULTIPLY_HPP_INCLUDED
