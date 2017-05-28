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
//! @file   HydraulicSymmetricCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-09-02
//!
//! @brief A symmetric piston component of C-type. Inherits HydraulicCylinderC.
//!
//$Id$

#ifndef HYDRAULICSYMMETRICCYLINDERC_H
#define HYDRAULICSYMMETRICCYLINDERC_H


#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HydraulicCylinderC.hpp"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicSymmetricCylinderC : public HydraulicCylinderC
{

    private:
        double *mpA;

    public:
        static Component *Creator()
        {
            return new HydraulicSymmetricCylinderC();
        }

        void configure()
        {
            HydraulicCylinderC::configure();

            removePort("A_1");
            removePort("A_2");

            addInputVariable("A", "Piston Area", "m^2", 0.001, &mpA);
        }

        void initialize()
        {
            mpA1 = mpA;
            mpA2 = mpA;
            HydraulicCylinderC::initialize();
        }

        void simulateOneTimestep()
        {
            HydraulicCylinderC::simulateOneTimestep();
        }
    };
}

#endif // HYDRAULICSYMMETRICCYLINDERC_H
