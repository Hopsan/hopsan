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
//! @file   SignalOr.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-19
//!
//! @brief Contains a logical and operator
//!
//$Id$

#ifndef SIGNALOR_HPP_INCLUDED
#define SIGNALOR_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalOr : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalOr();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0.0, &mpND_in1);
            addInputVariable("in2", "", "", 0.0, &mpND_in2);
            addOutputVariable("out", "", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Or operator equation
            (*mpND_out) = boolToDouble(doubleToBool(*mpND_in1) || doubleToBool(*mpND_in2));
        }
    };
}
#endif // SIGNALOR_HPP_INCLUDED
