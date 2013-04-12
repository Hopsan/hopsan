//!
//! @file   SignalNoiseGenerator.hpp
//! @author Robert Braun <robert.braun@liu.se
//! @date   2011-06-08
//!
//! @brief Contains a Signal Noise Generator Component
//!
//$Id$

#ifndef SIGNALNOISEGENERATOR_HPP_INCLUDED
#define SIGNALNOISEGENERATOR_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalNoiseGenerator : public ComponentSignal
    {

    private:
        WhiteGaussianNoise noise;
        double *mpOut, *mpStdDev;

    public:
        static Component *Creator()
        {
            return new SignalNoiseGenerator();
        }

        void configure()
        {
            addInputVariable("std_dev", "Amplitude Variance", "-", 1.0, &mpStdDev);
            addOutputVariable("out", "", "", 0.0, &mpOut);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
             (*mpOut) = (*mpStdDev)*noise.getValue();
        }
    };
}

#endif // SIGNALNOISEGENERATOR_HPP_INCLUDED
