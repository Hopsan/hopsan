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
//! @file   SignalTripleRoute.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2011-08-29
//!
//! @brief Contains a signal routing component with two inputs
//!
//$Id$

#ifndef SIGNALTRIPLEROUTE_HPP_INCLUDED
#define SIGNALTRIPLEROUTE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalTripleRoute : public ComponentSignal
    {

    private:
        double *mpND_route, *mpND_in1, *mpND_in2, *mpND_in3, *mpND_out;
        double limit12, limit23;

    public:
        static Component *Creator()
        {
            return new SignalTripleRoute();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addInputVariable("in3", "", "", 0, &mpND_in3);
            addInputVariable("route", "Input selection", "", 0, &mpND_route);
            addOutputVariable("out", "Selected input", "", &mpND_out);

            addConstant("limit12", "Limit value between input 1 and 2", "", 0.5, limit12);
            addConstant("limit23", "Limit value between input 2 and 3", "", 1.5, limit23);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            if((*mpND_route) < limit12 )
            {
                (*mpND_out) = (*mpND_in1);
            }
            else if(*mpND_route < limit23)
            {
                (*mpND_out) = (*mpND_in2);
            }
            else
            {
                (*mpND_out) = (*mpND_in3);
            }
        }
    };
}
#endif
