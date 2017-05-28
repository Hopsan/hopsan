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
//! @file   MechanicFixedPosition.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-11-09
//!
//! @brief Contains a Mechanic Fixed Position Component
//!
//$Id$

#ifndef MECHANICFIXEDPOSITION_HPP_INCLUDED
#define MECHANICFIXEDPOSITION_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class MechanicFixedPosition : public ComponentQ
    {

    private:
        double me;
        Port *mpPm1;
        double *mpPm1_f, *mpPm1_x, *mpPm1_v, *mpPm1_c, *mpPm1_me;

    public:
        static Component *Creator()
        {
            return new MechanicFixedPosition();
        }

        void configure()
        {
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");
            addConstant("m_e", "Equivalent Mass", "kg", 1, me);
        }

        void initialize()
        {
            mpPm1_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
            mpPm1_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
            mpPm1_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
            mpPm1_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
            mpPm1_me = getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);

            (*mpPm1_v) = 0;
            (*mpPm1_me) = me;
        }


        void simulateOneTimestep()
        {
            //Equations
            (*mpPm1_f) = (*mpPm1_c);
        }
    };
}

#endif // MECHANICFIXEDPOSITION_HPP_INCLUDED
