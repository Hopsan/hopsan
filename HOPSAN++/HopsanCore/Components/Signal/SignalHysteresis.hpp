//!
//! @file   SignalHysteresis.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-04
//!
//! @brief Contains a Signal Hysteresis Component
//!
//$Id$

#ifndef SIGNALHYSTERESIS_HPP_INCLUDED
#define SIGNALHYSTERESIS_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHysteresis : public ComponentSignal
    {

    private:
        double mHysteresisWidth;
        Delay mDelayedInput;
        ValveHysteresis mHyst;
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHysteresis("Hysteresis");
        }

        SignalHysteresis(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalHysteresis";
            mHysteresisWidth = 1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("HysteresisWidth", "Width of the Hysteresis", "-", mHysteresisWidth);
        }


        void initialize()
        {
            input = mpIn->getNodeDataPtr(NodeSignal::VALUE);

            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double();
            }

            mDelayedInput.initialize(1, 0.0);
        }


        void simulateOneTimestep()
        {
            //Hysteresis equations
            (*output) = mHyst.getValue((*input), mHysteresisWidth, mDelayedInput.getOldest());
            mDelayedInput.update((*output));
        }
    };
}

#endif // SIGNALHYSTERESIS_HPP_INCLUDED
