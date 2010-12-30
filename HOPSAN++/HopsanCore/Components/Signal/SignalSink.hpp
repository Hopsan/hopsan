//!
//! @file   SignalSink.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Sink Component
//!
//$Id$

#ifndef SIGNALSINK_HPP_INCLUDED
#define SIGNALSINK_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSink : public ComponentSignal
    {

    private:
        Port *mpIn;

    public:
        static Component *Creator()
        {
            return new SignalSink("Sink");
        }

        SignalSink(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSink";

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            //Nothing to initilize
        }


        void simulateOneTimestep()
        {
            //Nothing to do
        }

        void finalize()
        {
            //Nothing to do
        }
    };
}

#endif // SIGNALSINK_HPP_INCLUDED
