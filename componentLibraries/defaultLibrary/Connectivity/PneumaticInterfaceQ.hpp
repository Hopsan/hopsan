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
//! @file   PneumaticInterfaceQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-01
//!
//! @brief Contains a pneumatic interface component of Q-type
//!
//$Id$

#ifndef PNEUMATICINTERFACEQ_HPP_INCLUDED
#define PNEUMATICINTERFACEQ_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup InterfaceComponents
    //!
    class PneumaticInterfaceQ : public ComponentQ
    {

    private:
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new PneumaticInterfaceQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodePneumatic");
        }

        void initialize()
        {
            //Interfacing is handled through readnode/writenode from the RT wrapper file
        }

        void simulateOneTimestep()
        {
            //Interfacing is handled through readnode/writenode from the RT wrapper file
        }
    };
}

#endif // PNEUMATICINTERFACEQ_HPP_INCLUDED
