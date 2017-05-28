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

#ifndef MECHANICTRANSLATIONALQSOURCE_HPP
#define MECHANICTRANSLATIONALQSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicTranslationalQSource : public ComponentQ
{

private:
    double *mpIn_f, *mpIn_v, *mpIn_x, *mpIn_em;
    Port *mpP1;
    MechanicNodeDataPointerStructT mP1;

public:
    static Component *Creator()
    {
        return new MechanicTranslationalQSource();
    }

    void configure()
    {
        addInputVariable("in_f", "Force variable input", "Force", 0, &mpIn_f);
        addInputVariable("in_v", "Velocity variable input", "Velocity", 0, &mpIn_v);
        addInputVariable("in_x", "Position variable input", "Position", 0, &mpIn_x);
        addInputVariable("in_em", "Equivalent mass variable input", "Mass", 1, &mpIn_em);
        mpP1 = addPowerPort("P1", "NodeMechanic");
    }

    void initialize()
    {
        getMechanicPortNodeDataPointers(mpP1, mP1);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        mP1.rf() = readSignal(mpIn_f);
        mP1.rv() = readSignal(mpIn_v);
        mP1.rx() = readSignal(mpIn_x);
        mP1.rMe() = readSignal(mpIn_em);
    }
};
}

#endif // MECHANICTRANSLATIONALQSOURCE_HPP

