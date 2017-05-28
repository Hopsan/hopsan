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
//! @file   SignalStaircase.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-16
//!
//! @brief Contains a signal staircase function source
//!
//$Id$

#ifndef SIGNALSTAIRCASE_HPP_INCLUDED
#define SIGNALSTAIRCASE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalStaircase : public ComponentSignal
{
    private:
        double *mpStartT, *mpStepHeight, *mpStepWidth, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalStaircase();
        }

        void configure()
        {
            // Register changeable parameters to the HOPSAN++ core
            addInputVariable("T_start", "Start Time", "Time", 0.0, &mpStartT);
            addInputVariable("H_step", "Step Height", "", 1.0, &mpStepHeight);
            addInputVariable("W_step", "Step Width", "Time", 1.0, &mpStepWidth);

            // Add ports to the component, (the defaulvalue will be the base level and is changable as parameter)
            addOutputVariable("out", "Stair case output", "", 0.0, &mpOut);
        }


        void initialize()
        {
            // No startvale calculation, value from startvalue in outpor will be used
        }

        void simulateOneTimestep()
        {
            // +0.5*min(mtimestep,stepWidth) to avoid double!=int nummeric accuracy issue
            (*mpOut) = (*mpStepHeight)*floor(std::max(0.0, mTime-(*mpStartT)+0.5*std::min(mTimestep,(*mpStepWidth)))/(*mpStepWidth));
        }
    };
}

#endif
