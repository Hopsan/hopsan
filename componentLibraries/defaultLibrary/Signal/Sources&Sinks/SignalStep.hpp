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
