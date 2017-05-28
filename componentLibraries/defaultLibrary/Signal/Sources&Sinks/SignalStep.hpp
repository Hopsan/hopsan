/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
            addOutputVariable("out", "Step output", "", &mpOut);

            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("t_step", "Step Time", "Time", 1.0, &mpStepTime);
        }


        void initialize()
        {
//            mpOut = getSafeNodeDataPtr("out", NodeSignal::Value);
//            mpBaseValue = getSafeNodeDataPtr("y_0", NodeSignal::Value);
//            mpAmplitude = getSafeNodeDataPtr("y_A", NodeSignal::Value);
//            mpStepTime = getSafeNodeDataPtr("t_step", NodeSignal::Value);

            // Set initial value
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Step Equations
            if (mTime < (*mpStepTime))
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
