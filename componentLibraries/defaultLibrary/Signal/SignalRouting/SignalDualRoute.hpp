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
//! @file   SignalDualRoute.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2011-08-29
//!
//! @brief Contains a signal routing component with two inputs
//!
//$Id$

#ifndef SIGNALDUALROUTE_HPP_INCLUDED
#define SIGNALDUALROUTE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDualRoute : public ComponentSignal
    {

    private:
        double *mpND_route;
        double *mpND_in1;
        double *mpND_in2;
        double *mpND_out;
        double limit;

    public:
        static Component *Creator()
        {
            return new SignalDualRoute();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addInputVariable("route", "Input selection", "", 0, &mpND_route);
            addOutputVariable("out", "Selected input", "", &mpND_out);

            addConstant("limit", "Limit value", "", 0.5, limit);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            if((*mpND_route) < limit )
            {
                (*mpND_out) = (*mpND_in1);
            }
            else
            {
                (*mpND_out) = (*mpND_in2);
            }
        }
    };
}
#endif
