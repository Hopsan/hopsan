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
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            mDelay.initialize(mTime, mStartY);
            mDelay.setTimeDelay(mTimeDelay, mTimestep, mStartY);
            //! @todo Write out values into node as well? (I think so) This is true for all components
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);
            mDelay.update(u);
            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, mDelay.value(u));
        }
    };
}

#endif // SIGNALTIMEDELAY_HPP_INCLUDED
