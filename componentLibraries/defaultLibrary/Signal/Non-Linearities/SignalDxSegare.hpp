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
//! @file   SignalDxSegare.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-06-03
//!
//! @brief Contains the derivative of the signal segment section area function component
//!

#ifndef SIGNALDXSEGARE_HPP_INCLUDED
#define SIGNALDXSEGARE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDxSegare : public ComponentSignal
    {

    private:
        double *mpND_x, *mpND_width, *mpND_diameter;

    public:
        static Component *Creator()
        {
            return new SignalDxSegare();
        }

        void configure()
        {
            addInputVariable("x", "", "", 0.0, &mpND_x);
            addInputVariable("diameter", "", "", 0.0, &mpND_diameter);
            addOutputVariable("width", "dxsegare(in)","",&mpND_width);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_width) = dxSegare(*mpND_x,*mpND_diameter);
        }
    };
}

#endif // SIGNALDXSEGARE_HPP_INCLUDED
