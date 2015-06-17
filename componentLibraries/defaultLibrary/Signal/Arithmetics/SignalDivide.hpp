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
//! @file   SignalDivide.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical division function
//!
//$Id$

#ifndef SIGNALDIVIDE_HPP_INCLUDED
#define SIGNALDIVIDE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDivide : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDivide();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addOutputVariable("out", "in1/in2", "", &mpND_out);
        }


        void initialize()
        {
            // We do a weaker check for division be zero at first time step, to avoid initial value troubles.
            // Simulation is allowed to continue and output value is set to zero.
            // User gets a warning message.
            if(*mpND_in2 == 0)
            {
                addWarningMessage("Division by zero at first time step. Output value set to zero.");
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in1) / (*mpND_in2);
            }
        }

        void simulateOneTimestep()
        {
            // Stop simulation if division by zero.
            if(*mpND_in2 == 0)
            {
                addErrorMessage("Division by zero.");
                stopSimulation();
            }
            (*mpND_out) = (*mpND_in1) / (*mpND_in2);
        }
    };
}

#endif // SIGNALDIVIDE_HPP_INCLUDED
