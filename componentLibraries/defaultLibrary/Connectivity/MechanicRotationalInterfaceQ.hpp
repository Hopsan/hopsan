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
//! @file   MechanicRotationalInterfaceQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-30
//!
//! @brief Contains a rotational mechanic interface component of Q-type
//!
//$Id$

#ifndef MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED
#define MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup InterfaceComponents
    //!
    class MechanicRotationalInterfaceQ : public ComponentQ
    {

    private:
        Port *mpP1;
        double *mpND_t, *mpND_w, *mpND_c, *mpND_Zx;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInterfaceQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        }

        void initialize()
        {
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);
        }

        void simulateOneTimestep()
        {
            //! @todo If this works, do same in other Q-type interface components

            //Calculate torque from c and Zx
            double w = (*mpND_w);
            double c = (*mpND_c);
            double Zx = (*mpND_Zx);

            (*mpND_t) = c + Zx*w;
        }
    };
}

#endif // MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED




