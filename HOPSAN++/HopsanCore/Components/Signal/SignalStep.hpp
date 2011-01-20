//!
//! @file   SignalStep.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a step signal generator
//!
//$Id$

///////////////////////////////////////////
//                    XXXXXX  ↑          //
//                    X       | StepSize //
//                    X       |          //
// StartValue →  XXXXXX       ↓          //
//                                       //
//                    ↑                  //
//                 StepTime              //
///////////////////////////////////////////

#ifndef SIGNALSTEP_HPP_INCLUDED
#define SIGNALSTEP_HPP_INCLUDED

#include "../../ComponentEssentials.h"

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
            return new SignalStep("Step");
        }

        SignalStep(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalStep";
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mStepTime = 1.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("BaseValue", "Base Value", "-", mBaseValue);
            registerParameter("Amplitude", "Amplitude", "-", mAmplitude);
            registerParameter("StepTime", "Step Time", "-", mStepTime);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
            //Step Equations
            if (mTime <= mStepTime)
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
