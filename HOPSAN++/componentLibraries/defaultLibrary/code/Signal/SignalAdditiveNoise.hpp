//!
//! @file   SignalAdditiveNoise.hpp
//! @author Robert Braun <robert.braun@liu.se
//! @date   2011-06-08
//!
//! @brief Contains a Signal Noise Generator Component
//!
//$Id$

#ifndef SIGNALADDITIVENOISE_HPP_INCLUDED
#define SIGNALADDITIVENOISE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAdditiveNoise : public ComponentSignal
    {

    private:
        double y;
        WhiteGaussianNoise noise;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalAdditiveNoise();
        }

        void configure()
        {
            y = 1.0;
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("sigma^2", "Amplitude Variance", "[-]", y);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, 0);
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
             (*mpND_out) = (*mpND_in) + y*noise.getValue();
        }
    };
}

#endif // SIGNALADDITIVENOISE_HPP_INCLUDED
