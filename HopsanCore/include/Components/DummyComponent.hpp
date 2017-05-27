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
//! @file   DummyComponent.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-10-25
//!
//! @brief Contains an empty dummy component
//!
//$Id$

#ifndef DUMMYCOMPONENT_HPP_INCLUDED
#define DUMMYCOMPONENT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class DummyComponent : public ComponentSignal
    {
    private:

    public:
        static Component *Creator()
        {
            return new DummyComponent();
        }

        void configure()
        {
            // Do nothing
        }


        void initialize()
        {
            // Do nothing
        }


        void simulateOneTimestep()
        {
            // Do nothing
        }
    };
}

#endif
