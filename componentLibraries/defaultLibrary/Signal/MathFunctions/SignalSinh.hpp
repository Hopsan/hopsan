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
//! @file   SignalSinh.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-04-12
//!
//! @brief Contains a signal sinh function component
//!

#ifndef SIGNALSINH_HPP_INCLUDED
#define SIGNALSINH_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSinh : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalSinh();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "Sinh(in)","",&mpND_out);
           }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = sinh(*mpND_in);
        }
    };
}

#endif // SIGNALSINH_HPP_INCLUDED
