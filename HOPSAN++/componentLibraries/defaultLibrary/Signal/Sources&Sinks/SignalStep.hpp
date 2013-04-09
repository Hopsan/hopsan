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
//! @file   SignalStep.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a step signal generator
//!
//$Id$

///////////////////////////////////////////
//                    XXXXXX  â          //
//                    X       | StepSize //
//                    X       |          //
// StartValue â  XXXXXX       â          //
//                                       //
//                    â                  //
//                 StepTime              //
///////////////////////////////////////////

#ifndef SIGNALSTEP_HPP_INCLUDED
#define SIGNALSTEP_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStep : public ComponentSignal
    {

    private:
        double *mpOut, *mpBaseValue, *mpAmplitude, *mpStepTime;

    public:
        static Component *Creator()
        {
            return new SignalStep();
        }

        void configure()
        {
            addOutputVariable("out", "Step output", "-");

            addInputVariable("y_0", "Base Value", "-", 0.0);
            addInputVariable("y_A", "Amplitude", "-", 1.0);
            addInputVariable("t_step", "Step Time", "-", 1.0);
        }


        void initialize()
        {
            mpOut = getSafeNodeDataPtr("out", NodeSignal::Value);
            mpBaseValue = getSafeNodeDataPtr("y_0", NodeSignal::Value);
            mpAmplitude = getSafeNodeDataPtr("y_A", NodeSignal::Value);
            mpStepTime = getSafeNodeDataPtr("t_step", NodeSignal::Value);

            // Set initial value
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Step Equations
            if (mTime < *mpStepTime)
            {
                (*mpOut) = (*mpBaseValue);     //Before step
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);     //After step
            }
        }
    };
}

#endif // SIGNALSTEP_HPP_INCLUDED
