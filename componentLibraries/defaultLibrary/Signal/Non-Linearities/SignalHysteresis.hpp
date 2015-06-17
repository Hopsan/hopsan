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
//! @file   SignalHysteresis.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-04
//!
//! @brief Contains a Signal Hysteresis Component
//!
//$Id$

#ifndef SIGNALHYSTERESIS_HPP_INCLUDED
#define SIGNALHYSTERESIS_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHysteresis : public ComponentSignal
    {

    private:
        double *mpHysteresisWidth;
        Delay mDelayedInput;
        ValveHysteresis mHyst;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalHysteresis();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addInputVariable("y_h", "Width of the Hysteresis", "", 1.0, &mpHysteresisWidth);

            addOutputVariable("out", "", "", &mpND_out);
        }


        void initialize()
        {
            mDelayedInput.initialize(1, (*mpND_in));
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            //Hysteresis equations
            (*mpND_out) = mHyst.getValue((*mpND_in), (*mpHysteresisWidth), mDelayedInput.getOldest());
            mDelayedInput.update((*mpND_out));
        }
    };
}

#endif // SIGNALHYSTERESIS_HPP_INCLUDED
