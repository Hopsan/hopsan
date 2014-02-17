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
        double *mpBaseValue;
        double *mpAmplitude;
        double *mpStepTime;
        double *mpTimeConstant;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStepExponentialDelay();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "-", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "-", 1.0, &mpAmplitude);
            addInputVariable("tao", "Time Constant of Delay", "-", 1.0, &mpTimeConstant);
            addInputVariable("t_step", "Step Time", "-", 1.0, &mpStepTime);

            addOutputVariable("out", "", "", &mpOut);
        }


        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            // StepExponentialDelay Equations
            if (mTime < (*mpStepTime))
            {
                (*mpOut) = (*mpBaseValue);     //Before Step
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude) * exp(-(mTime-(*mpStepTime))/(*mpTimeConstant));     //After Step
            }
        }
    };
}

#endif // SIGNALSTEPEXPONENTIALDELAY_HPP_INCLUDED
