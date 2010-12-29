//!
//! @file   SignalDelay.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a Signal Delay Function using CoreUtilities
//!
//$Id$

#ifndef SIGNALDELAY_HPP_INCLUDED
#define SIGNALDELAY_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDelay : public ComponentSignal
    {

    private:
        Delay mDelay;
        double mDt;
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDelay("Delay");
        }

        SignalDelay(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalDelay";

            mDt=1;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("dt", "Delay time", "s", mDt);
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

            mDelay.initialize(mDt, mTimestep, mpOut->getStartValue(NodeSignal::VALUE));
        }


        void simulateOneTimestep()
        {
            //Write new values to nodes
            (*output) = mDelay.update((*input));
        }
    };
}

#endif // SIGNALDELAY_HPP_INCLUDED
