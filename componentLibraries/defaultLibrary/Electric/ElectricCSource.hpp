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

#ifndef ElectricCSOURCE_HPP
#define ElectricCSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup ElectricComponents
//!
class ElectricCSource : public ComponentC
{

private:
    double *mpIn_c, *mpIn_Zx;
    double *mpP1_c, *mpP1_Zx;

public:
    static Component *Creator()
    {
        return new ElectricCSource();
    }

    void configure()
    {
        addInputVariable("in_c", "Wave variable input", "Voltage", 0, &mpIn_c);
        addInputVariable("in_z", "Char. impedance variable input", "N s/m", 0, &mpIn_Zx);
        addPowerPort("P1", "NodeElectric");
    }

    void initialize()
    {
        mpP1_c = getSafeNodeDataPtr("P1", NodeElectric::WaveVariable);
        mpP1_Zx = getSafeNodeDataPtr("P1", NodeElectric::CharImpedance);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        (*mpP1_c) = (*mpIn_c);
        (*mpP1_Zx) = (*mpIn_Zx);
    }
};
}

#endif // ElectricCSOURCE_HPP

