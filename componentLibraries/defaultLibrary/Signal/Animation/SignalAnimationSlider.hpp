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
//! @file   SignalGain.hpp
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-05-07
//!
//! @brief Contains a Signal Adjustable Slider (does nothing in the simulation)
//!
//$Id$

#ifndef SIGNALANIMATIONSLIDER_HPP_INCLUDED
#define SIGNALANIMATIONSLIDER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAnimationSlider : public ComponentSignal
    {

    private:
        double* mpIn;
        double* mpOut;
        double mMin;
        double mMax;

    public:
        static Component *Creator()
        {
            return new SignalAnimationSlider();
        }

        void configure()
        {
            addInputVariable("in", "Input signal (between 0 and 1)", "", 0, &mpIn);
            addOutputVariable("out", "Output signal", "", 0, &mpOut);
            addConstant("min", "Minimum input value", "", "", 0, mMin);
            addConstant("max", "Minimum output value", "", "", 1, mMax);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mMin + (mMax - mMin)*(*mpIn);
        }
    };
}

#endif //SIGNALANIMATIONSLIDER_HPP_INCLUDED
