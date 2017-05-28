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
//! @file   MechanicTorsionalSpring.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a torsional spring
//!
//$Id$

#ifndef MECHANICTORSIONALSPRING_HPP_INCLUDED
#define MECHANICTORSIONALSPRING_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorsionalSpring : public ComponentC
    {

    private:
        double k;
        double w1, c1, lastc1, w2, c2, lastc2, Zx;
        double *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_w2, *mpND_c2, *mpND_Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTorsionalSpring();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            addConstant("k", "Spring Coefficient", "Nm/rad", 100.0, k);
        }


        void initialize()
        {
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            Zx = k*mTimestep;

            (*mpND_Zx1) = Zx;
            (*mpND_Zx2) = Zx;

        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            w1 = (*mpND_w1);
            lastc1 = (*mpND_c1);
            w2 = (*mpND_w2);
            lastc2 = (*mpND_c2);

            //Spring equations
            c1 = lastc2 + 2.0*Zx*w2;
            c2 = lastc1 + 2.0*Zx*w1;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_c2) = c2;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


