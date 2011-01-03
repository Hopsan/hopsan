//!
//! @file   SignalGain.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id$

#ifndef SIGNALGAIN_HPP_INCLUDED
#define SIGNALGAIN_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGain : public ComponentSignal
    {

    private:
        double mGain;
        Port *mpIn, *mpOut;

        double *input, *output;

    public:
        static Component *Creator()
        {
            return new SignalGain("Gain");
        }

        SignalGain(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalGain";
            mGain = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("Gain", "Gain value", "-", mGain);
        }


        void initialize()
        {
            if(mpIn->isConnected())
            {
                input = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input = new double(0);
            }

            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double();
            }
        }


        void simulateOneTimestep()
        {
            (*output) = mGain * (*input);
        }
    };
}

#endif // SIGNALGAIN_HPP_INCLUDED
