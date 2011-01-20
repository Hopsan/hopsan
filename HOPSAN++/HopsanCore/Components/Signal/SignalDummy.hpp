//!
//! @file   SignalGain.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id: SignalGain.hpp 2008 2010-10-26 08:43:52Z robbr48 $

#ifndef SIGNALDUMMY_HPP_INCLUDED
#define SIGNALDUMMY_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDummy : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;

        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDummy("Dummy");
        }

        SignalDummy(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalDummy";

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = 1;
            for(size_t i=0; i<(*mpND_in); ++i)
            {
                (*mpND_out) = (*mpND_out) * i;
            }
        }
    };
}

#endif // SIGNALDUMMY_HPP_INCLUDED
