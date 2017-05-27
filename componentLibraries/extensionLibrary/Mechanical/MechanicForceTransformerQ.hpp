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

#ifndef MECHANICFORCETRANSFORMERQ_HPP_INCLUDED
#define MECHANICFORCETRANSFORMERQ_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities/Integrator.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicForceTransformerQ : public ComponentQ
{

private:
    MechanicNodeDataPointerStructT mP1;
    double *mpF_signal;
    Integrator mIntegrator1;

    Port *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicForceTransformerQ();
    }

    void configure()
    {
        addInputVariable("F", "Generated force", "N", 0.0, &mpF_signal);
        mpP1 = addPowerPort("P1", "NodeMechanic");
        disableStartValue(mpP1, NodeMechanic::Force);
    }


    void initialize()
    {
        getMechanicPortNodeDataPointers(mpP1, mP1);
        mIntegrator1.initialize(mTimestep, mP1.v(), mP1.x());
        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        // First calculate the non-contact behavior
        const double f = readSignal(mpF_signal);
        mP1.rf() = f;
        mP1.rv() = (f-mP1.c())/std::max(mP1.Zc(), 1e-12);
        mP1.rx() = mIntegrator1.update(mP1.v());
    }
};
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
