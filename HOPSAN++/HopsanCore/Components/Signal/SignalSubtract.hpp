//!
//! @file   SignalSubtract.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical subtraction function
//!
//$Id$

#ifndef SIGNALSUBTRACT_HPP_INCLUDED
#define SIGNALSUBTRACT_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSubtract : public ComponentSignal
    {

    private:
        double *input1, *input2, *output;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSubtract("Subtract");
        }

        SignalSubtract(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSubtract";

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            if(mpIn1->isConnected())
            {
                input1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input1 = new double(0);
            }

            if(mpIn2->isConnected())
            {
                input2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
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

            (*output) = 0;

        }


        void simulateOneTimestep()
        {
            //Subtract equations
            (*output) = (*input1) - (*input2);
        }
    };
}

#endif // SIGNALSUBTRACT_HPP_INCLUDED

