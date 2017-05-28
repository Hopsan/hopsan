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
