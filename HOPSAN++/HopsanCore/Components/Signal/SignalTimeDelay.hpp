//!
//! @file   SignalTimeDelay.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Time Delay Component
//!
//$Id$

#ifndef SIGNALTIMEDELAY_HPP_INCLUDED
#define SIGNALTIMEDELAY_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalTimeDelay : public ComponentSignal
    {

    private:
        double mStartY, mTimeDelay;
        Delay mDelay;
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalTimeDelay("TimeDelay");
        }

        SignalTimeDelay(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalTimeDelay";
            mStartY = 0.0;
            mTimeDelay = 1.0;

            registerParameter("TD", "Time delay", "s", mTimeDelay);

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
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

            mDelay.initialize(mTimeDelay, mTimestep, mStartY);
            (*output) = mStartY;
        }


        void simulateOneTimestep()
        {
            (*output) =  mDelay.update(*input);
        }
    };
}

#endif // SIGNALTIMEDELAY_HPP_INCLUDED
