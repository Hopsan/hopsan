/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   SignalQuadRoute.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2011-08-29
//!
//! @brief Contains a signal routing component with two inputs
//!
//$Id$

#ifndef SIGNALQUADROUTE_HPP_INCLUDED
#define SIGNALQUADROUTE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalQuadRoute : public ComponentSignal
    {

    private:
        double *mpND_route, *mpND_in1, *mpND_in2, *mpND_in3, *mpND_in4, *mpND_out;
        double limit12, limit23, limit34;

    public:
        static Component *Creator()
        {
            return new SignalQuadRoute();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addInputVariable("in3", "", "", 0, &mpND_in3);
            addInputVariable("in4", "", "", 0, &mpND_in4);
            addInputVariable("route", "Input selection", "", 0, &mpND_route);
            addOutputVariable("out", "Selected input", "", &mpND_out);

            addConstant("limit12", "Limit value between input 1 and 2", "", 0.5, limit12);
            addConstant("limit23", "Limit value between input 2 and 3", "", 1.5, limit23);
            addConstant("limit34", "Limit value between input 3 and 4", "", 2.5, limit34);
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
            else if((*mpND_route) < limit23 )
            {
                (*mpND_out) = (*mpND_in2);
            }
            else if((*mpND_route) < limit34 )
            {
                (*mpND_out) = (*mpND_in3);
            }
            else
            {
                (*mpND_out) = (*mpND_in4);
            }
        }
    };
}
#endif
