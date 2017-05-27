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
//! @file   MechanicAngleSensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-12
//!
//! @brief Contains a Mechanic Angle Sensor Component
//!
//$Id$

#ifndef MECHANICANGLESENSOR_HPP_INCLUDED
#define MECHANICANGLESENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngleSensor : public ComponentSignal
    {
    private:
        double *mpP1_a, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicAngleSensor();
        }

        void configure()
        {
            addReadPort("P1", "NodeMechanicRotational", "", Port::NotRequired);
            addOutputVariable("out", "Angle", "Angle", &mpOut);
        }


        void initialize()
        {
            mpP1_a = getSafeNodeDataPtr("P1", NodeMechanicRotational::Angle);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = (*mpP1_a);
        }
    };
}

#endif // MECHANICANGLESENSOR_HPP_INCLUDED
