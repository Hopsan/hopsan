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
//! @file   MechanicNodeSensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-11-17

//$Id$

#ifndef MECHANICNODESENSOR_HPP
#define MECHANICNODESENSOR_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicNodeSensor : public ComponentSignal
{
private:
    MechanicNodeDataPointerStructT mP1;
    double *mpOut_f, *mpOut_v, *mpOut_x, *mpOut_c, *mpOut_z, *mpOut_em;

public:
    static Component *Creator()
    {
        return new MechanicNodeSensor();
    }

    void configure()
    {
        addReadPort("P1", "NodeMechanic", "Sensor port", Port::NotRequired);
        addOutputVariable("out_f", "Force", "Force", &mpOut_f);
        addOutputVariable("out_v", "Velocity", "Velocity", &mpOut_v);
        addOutputVariable("out_x", "Position", "Position", &mpOut_x);
        addOutputVariable("out_c", "Wave variable", "Force", &mpOut_c);
        addOutputVariable("out_z", "Char. impedance", "", &mpOut_z);
        addOutputVariable("out_em", "Equivalent mass", "Mass", &mpOut_em);
    }

    void initialize()
    {
        getMechanicPortNodeDataPointers(getPort("P1"), mP1);
        simulateOneTimestep(); //Set initial output node value
    }

    void simulateOneTimestep()
    {
        writeOutputVariable(mpOut_f, mP1.f());
        writeOutputVariable(mpOut_v, mP1.v());
        writeOutputVariable(mpOut_x, mP1.x());
        writeOutputVariable(mpOut_c, mP1.c());
        writeOutputVariable(mpOut_z, mP1.Zc());
        writeOutputVariable(mpOut_em, mP1.me());
    }
};
}

#endif // MECHANICNODESENSOR_HPP

