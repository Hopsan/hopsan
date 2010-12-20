//!
//! @file   SignalShuttleValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-17
//!
//! @brief Contains a Shuttle Valve component
//!
//$Id$

#ifndef SIGNALSHUTTLEVALVE_HPP_INCLUDED
#define SIGNALSHUTTLEVALVE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalShuttleValve : public ComponentSignal
    {

    private:
        double mIn1, mIn2;
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalShuttleValve("ShuttleValve");
        }

        SignalShuttleValve(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalShuttleValve";

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            mIn1 = mpIn1->readNode(NodeSignal::VALUE);
            mIn2 = mpIn1->readNode(NodeSignal::VALUE);

            if(mIn1 > mIn2)
            {
                mpOut->writeNode(NodeSignal::VALUE, mIn1);
            }
            else
            {
                mpOut->writeNode(NodeSignal::VALUE, mIn2);
            }
        }
    };
}

#endif // SIGNALGREATERTHAN_HPP_INCLUDED
