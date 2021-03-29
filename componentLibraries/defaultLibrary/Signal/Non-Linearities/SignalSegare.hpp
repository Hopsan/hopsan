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
//! @file   SignalSegare.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date   2015-03-07
//!
//! @brief Contains a signal segment section area function component
//!

#ifndef SIGNALSEGARE_HPP_INCLUDED
#define SIGNALSEGARE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <math.h>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSegare : public ComponentSignal
    {

    private:
        double *mpND_x, *mpND_area, *mpND_diameter;

    public:
        static Component *Creator()
        {
            return new SignalSegare();
        }

        void configure()
        {
            addInputVariable("x", "Segment height", "", 0.0, &mpND_x);
            addInputVariable("diameter", "Circle diameter", "", 0.0, &mpND_diameter);
            addOutputVariable("area", "Segment area","",&mpND_area);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_area) = segare(*mpND_x,*mpND_diameter);
        }
    };
}

#endif // SIGNALSEGARE_HPP_INCLUDED
