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

#ifndef HydraulicQSOURCE_HPP
#define HydraulicQSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicQSource : public ComponentQ
{

private:
    double *mpIn_p, *mpIn_q;
    Port *mpP1;
    HydraulicNodeDataPointerStructT mP1;

public:
    static Component *Creator()
    {
        return new HydraulicQSource();
    }

    void configure()
    {
        addInputVariable("in_p", "Pressure variable input", "Pressure", 0, &mpIn_p);
        addInputVariable("in_q", "Flow variable input", "Flow", 0, &mpIn_q);
        mpP1 = addPowerPort("P1", "NodeHydraulic");
    }

    void initialize()
    {
        getHydraulicPortNodeDataPointers(mpP1, mP1);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        mP1.rp() = readSignal(mpIn_p);
        mP1.rq() = readSignal(mpIn_q);
    }
};
}

#endif

