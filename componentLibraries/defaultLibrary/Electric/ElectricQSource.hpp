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


//$Id$

#ifndef ElectricQSOURCE_HPP
#define ElectricQSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup ElectricComponents
//!
class ElectricQSource : public ComponentQ
{

private:
    double *mpIn_u, *mpIn_i;
    Port *mpP1;
    ElectricNodeDataPointerStructT mP1;

public:
    static Component *Creator()
    {
        return new ElectricQSource();
    }

    void configure()
    {
        addInputVariable("in_u", "Voltage variable input", "Voltage", 0, &mpIn_u);
        addInputVariable("in_i", "Current variable input", "Current", 0, &mpIn_i);
        mpP1 = addPowerPort("P1", "NodeElectric");
    }

    void initialize()
    {
        getElectricPortNodeDataPointers(mpP1, mP1);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        mP1.ru() = readSignal(mpIn_u);
        mP1.ri() = readSignal(mpIn_i);
    }
};
}

#endif

