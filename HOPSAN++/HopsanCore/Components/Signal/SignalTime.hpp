//!
//! @file   SignalSource.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-03
//!
//! @brief Contains a Signal Time Component
//!
//$Id$

#ifndef SIGNALTIME_HPP_INCLUDED
#define SIGNALTIME_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalTime : public ComponentSignal
    {

    private:
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalTime("Time");
        }

        SignalTime(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalTime";

            mpOut = addWritePort("out", "NodeSignal");

        }


        void initialize()
        {
            mpOut->writeNode(NodeSignal::VALUE, 0.0);
        }


        void simulateOneTimestep()
        {
            mpOut->writeNode(NodeSignal::VALUE, mTime);
        }
    };
}

#endif // SIGNALTIME_HPP_INCLUDED
