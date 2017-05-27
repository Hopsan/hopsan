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

#ifndef HYDRAULICDUMMYQ_HPP_INCLUDED
#define HYDRAULICDUMMYQ_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    class HydraulicDummyQ : public ComponentQ
    {
    private:
        Port *mpP2;
        double *mpIn, *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1;

    public:
        static Component *Creator()
        {
            return new HydraulicDummyQ();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            mpP2 = addPowerPort("P1", "NodeHydraulic","",Port::NotRequired);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            (*mpND_p1) = 1;
            for(int i=0; i<(*mpIn); ++i)
            {
                (*mpND_p1) = (*mpND_p1) * i;
            }
        }
    };
}

#endif // HYDRAULICDUMMYQ_HPP_INCLUDED
