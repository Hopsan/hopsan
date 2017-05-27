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
//! @file   MechanicFixedPositionMultiPort.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-21
//!
//! @brief Contains a Mechanic Fixed Position Component
//!
//$Id$

#ifndef MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED
#define MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class MechanicFixedPositionMultiPort : public ComponentQ
    {

    private:
        double me;
        Port *mpMP;
        std::vector<double*> mvPm1_f;
        std::vector<double*> mvPm1_x;
        std::vector<double*> mvPm1_v;
        std::vector<double*> mvPm1_c;
        std::vector<double*> mvPm1_Zc;
        std::vector<double*> mvPm1_me;
        size_t mNumPorts;

    public:
        static Component *Creator()
        {
            return new MechanicFixedPositionMultiPort();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("Pm1", "NodeMechanic");

            addConstant("m_e", "Equivalent Mass", "kg", 1, me);
        }

        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();

            //! @todo write help function to set the size and contents of a these vectors automatically
            mvPm1_f.resize(mNumPorts);
            mvPm1_x.resize(mNumPorts);
            mvPm1_v.resize(mNumPorts);
            mvPm1_c.resize(mNumPorts);
            mvPm1_Zc.resize(mNumPorts);
            mvPm1_me.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvPm1_f[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Force);
                mvPm1_x[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Position);
                mvPm1_v[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Velocity);
                mvPm1_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::WaveVariable);
                mvPm1_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::CharImpedance);
                mvPm1_me[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::EquivalentMass);

                *(mvPm1_v[i]) = 0;
                *(mvPm1_me[i]) = me;
            }
        }


        void simulateOneTimestep()
        {
            //Equations
            for(size_t i=0; i<mNumPorts; ++i)
            {
                *(mvPm1_f[i]) = *(mvPm1_c[i]);
            }
        }
    };
}

#endif // MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED
