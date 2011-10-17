/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SignalStepExponentialDelay.hpppp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-08-01
//!
//! @brief Contains a Step With Exponential Delay signal generator
//!
//$Id$

#ifndef SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED
#define SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStepExponentialDelay : public ComponentSignal
    {

    private:
        double mBaseValue;
        double mAmplitude;
        double mStepTime;
        double mTimeConstant;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStepExponentialDelay("StepExponentialDelay");
        }

        SignalStepExponentialDelay(const std::string name) : ComponentSignal(name)
        {
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mStepTime = 1.0;
            mTimeConstant = 1.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("y_0", "Base Value", "[-]", mBaseValue);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);
            registerParameter("tao", "Time Constant of Delay", "[-]", mTimeConstant);
            registerParameter("t_step", "Step Time", "[-]", mStepTime);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
            //StepExponentialDelay Equations
            if (mTime <= mStepTime)
            {
                (*mpND_out) = mBaseValue;     //Before Step
            }
            else
            {
                (*mpND_out) = mBaseValue + mAmplitude * exp(-(mTime-mStepTime)/mTimeConstant);     //After Step
            }
        }
    };
}

#endif // SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED
