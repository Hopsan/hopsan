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

        WhiteGaussianNoise noise;
        double *mpND_in, *mpND_out, *mpND_stdDev;

    public:
        static Component *Creator()
        {
            return new SignalAdditiveNoise();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addInputVariable("std_dev", "Amplitude Variance", "", 1.0, &mpND_stdDev);

            addOutputVariable("out", "", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
             (*mpND_out) = (*mpND_in) + (*mpND_stdDev)*noise.getValue();
        }
    };
}

#endif // SIGNALADDITIVENOISE_HPP_INCLUDED
