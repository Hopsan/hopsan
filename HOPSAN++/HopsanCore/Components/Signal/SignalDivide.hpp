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
        double *input1, *input2, *output;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDivide("Divide");
        }

        SignalDivide(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalDivide";

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            if (mpIn1->isConnected())
            {
                input1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input1 = new double(0);     //Output shall be zero if no nominator input
            }

            if (mpIn2->isConnected())
            {
                input2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input2 = new double(1);     //Divide by one if no denominator input
            }

            if (mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double(0);     //Create a dummy pointer if output not connected
            }
        }


        void simulateOneTimestep()
        {
            //Divide equation
            (*output) = (*input1) / (*input2);
        }
    };
}

#endif // SIGNALDIVIDE_HPP_INCLUDED
