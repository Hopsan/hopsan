//!
//! @file   SignalOr.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-19
//!
//! @brief Contains a logical and operator
//!
//$Id$

#ifndef SIGNALOR_HPP_INCLUDED
#define SIGNALOR_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalOr : public ComponentSignal
    {

    private:
        double *input1, *input2, *output;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalOr("Or");
        }

        SignalOr(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalOr";

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
            //Or operator equation
            (*output) = boolToDouble(doubleToBool(*input1) || doubleToBool(*input2));
        }
    };
}
#endif // SIGNALOR_HPP_INCLUDED
