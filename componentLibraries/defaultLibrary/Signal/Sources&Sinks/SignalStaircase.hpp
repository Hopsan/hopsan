/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
