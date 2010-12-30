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
        double *output;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalTime("Time");
        }

        SignalTime(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalTime";

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

        }


        void initialize()
        {
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
            (*output) = mTime;
        }
    };
}

#endif // SIGNALTIME_HPP_INCLUDED
