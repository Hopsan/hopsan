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

        double *input, *output;

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
            input = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            if(mpOut->isConnected())
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            else
                output = new double(0);

            *output = 0.0;
        }


        void simulateOneTimestep()
        {
            (*output) = 1;
            for(size_t i=0; i<(*input); ++i)
            {
                (*output) = (*output) * i;
            }
        }
    };
}

#endif // SIGNALDUMMY_HPP_INCLUDED
