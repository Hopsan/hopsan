//!
//! @file   SignalAdd.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical summator component
//!
//$Id$

#ifndef SIGNALADD_HPP_INCLUDED
#define SIGNALADD_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAdd : public ComponentSignal
    {

    private:
        double *in1, *in2, *output;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalAdd("Add");
        }

        SignalAdd(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalAdd";

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
                //Make in1 or in2 be zero if they are not connected
            if(mpIn1->isConnected())
            {
                in1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                in1 = new double(0);
            }

            if(mpIn2->isConnected())
            {
                in2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                in2 = new double(0);
            }

                //output must be connected
            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double(0);
            }
        }


        void simulateOneTimestep()
        {
            (*output) = (*in1) + (*in2);
        }
    };
}
#endif // SIGNALADD_HPP_INCLUDED
