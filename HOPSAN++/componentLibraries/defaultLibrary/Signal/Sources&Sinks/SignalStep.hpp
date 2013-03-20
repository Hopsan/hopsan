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
        double mBaseValue;
        double mAmplitude;
        double mStepTime;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStep();
        }

        void configure()
        {
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mStepTime = 1.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("y_0", "Base Value", "[-]", mBaseValue);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);
            registerParameter("t_step", "Step Time", "[-]", mStepTime);

            disableStartValue(mpOut, NodeSignal::Value);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
            //Step Equations
            if (mTime < mStepTime)
            {
                (*mpND_out) = mBaseValue;     //Before step
            }
            else
            {
                (*mpND_out) = mBaseValue + mAmplitude;     //After step
            }
        }
    };
}

#endif // SIGNALSTEP_HPP_INCLUDED
