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
//! @file   SignalAtan2.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-04-12
//!
//! @brief Contains a signal atan2 function component
//!

#ifndef SIGNALATAN2_HPP_INCLUDED
#define SIGNALATAN2_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAtan2 : public ComponentSignal
    {

    private:
        double *mpND_inY, *mpND_inX, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalAtan2();
        }

        void configure()
        {
            addInputVariable("inY", "", "", 0.0, &mpND_inY);
            addInputVariable("inX", "", "", 0.0, &mpND_inX);
            addOutputVariable("out", "atan2(in)","",&mpND_out);
         }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) =atan2(*mpND_inY,*mpND_inX);
        }
    };
}

#endif // SIGNALATAN2_HPP_INCLUDED
