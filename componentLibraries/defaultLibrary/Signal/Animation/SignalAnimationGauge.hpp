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
//! @file   SignalAnimationGauge.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-17
//!
//! @brief Contains a Signal Animated Gauge (does nothing in the simulation)
//!
//$Id$

#ifndef SIGNALANIMATIONGAUGE_HPP_INCLUDED
#define SIGNALANIMATIONGAUGE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAnimationGauge : public ComponentSignal
    {

    private:
        double max;

    public:
        static Component *Creator()
        {
            return new SignalAnimationGauge();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0);
            addConstant("max", "Upper limit", "", 1, max);
        }


        void initialize()
        {

        }


        void simulateOneTimestep()
        {

        }
    };
}

#endif //SIGNALANIMATIONGAUGE_HPP_INCLUDED
