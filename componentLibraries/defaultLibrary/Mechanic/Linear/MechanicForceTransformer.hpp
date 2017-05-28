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

#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicForceTransformer : public ComponentC
{

private:
    double *mpF_signal, *mpP1_f, *mpP1_c;
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicForceTransformer();
    }

    void configure()
    {
        addInputVariable("F", "Generated force", "N", 0.0, &mpF_signal);
        mpP1 = addPowerPort("P1", "NodeMechanic");
        disableStartValue(mpP1, NodeMechanic::Force);
        setDefaultStartValue(mpP1, NodeMechanic::CharImpedance, 0);
        disableStartValue(mpP1, NodeMechanic::CharImpedance);
    }


    void initialize()
    {
        mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
        mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);

        (*mpP1_f) = (*mpF_signal);
        if ((*mpP1_c) == 0)
        {
            (*mpP1_c) = (*mpF_signal);
        }
    }


    void simulateOneTimestep()
    {
        (*mpP1_c) = (*mpF_signal);
    }
};
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
