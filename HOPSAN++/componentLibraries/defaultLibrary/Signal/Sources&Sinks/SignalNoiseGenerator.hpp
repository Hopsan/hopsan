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
        double y;
        WhiteGaussianNoise noise;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalNoiseGenerator();
        }

        void configure()
        {
            y = 1.0;
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("std_dev", "Amplitude Variance", "[-]", y);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, 0);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
             (*mpND_out) = y*noise.getValue();
        }
    };
}

#endif // SIGNALNOISEGENERATOR_HPP_INCLUDED
