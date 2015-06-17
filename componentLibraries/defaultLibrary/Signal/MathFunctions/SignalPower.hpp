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
//! @file   SignalPower.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2010-09-28
//!
//! @brief Contains a mathematical square component
//!
//$Id$

#ifndef SIGNALPOWER_HPP_INCLUDED
#define SIGNALPOWER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPower : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out, *mpX;

    public:
        static Component *Creator()
        {
            return new SignalPower();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addInputVariable("x", "Exponential", "", 0.0, &mpX);

            addOutputVariable("out", "in^x", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = std::pow(std::abs(*mpND_in), (*mpX));
        }
    };
}
#endif // SIGNALPOWER_HPP_INCLUDED
