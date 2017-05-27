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
//! @file   MechanicAngularVelocitySensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-22
//!
//! @brief Contains a Mechanic Angular Velocity Sensor Component
//!
//$Id$

#ifndef MECHANICANGULARVELOCITYSENSOR_HPP_INCLUDED
#define MECHANICANGULARVELOCITYSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngularVelocitySensor : public ComponentSignal
    {
    private:
        double *mpP1_w, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicAngularVelocitySensor();
        }

        void configure()
        {
            addReadPort("P1", "NodeMechanicRotational", "", Port::NotRequired);
            addOutputVariable("out", "AngularVelocity", "AngularVelocity", &mpOut);
        }


        void initialize()
        {
            mpP1_w = getSafeNodeDataPtr("P1", NodeMechanicRotational::AngularVelocity);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = (*mpP1_w);
        }
    };
}

#endif // MECHANICANGULARVELOCITYSENSOR_HPP_INCLUDED
