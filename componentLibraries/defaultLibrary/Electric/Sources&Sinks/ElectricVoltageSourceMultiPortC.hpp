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

#ifndef ELECTRICVOLTAGESOURCEMULTIPORTC_HPP
#define ELECTRICVOLTAGESOURCEMULTIPORTC_HPP

#include <vector>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup ElectricComponents
    //!
    class ElectricVoltageSourceMultiPortC : public ComponentC
    {
    private:
        Port *mpMP;
        size_t mNumPorts;
        std::vector<double*> mvpMP_u;
        std::vector<double*> mvpMP_i;
        std::vector<double*> mvpMP_c;
        std::vector<double*> mvpMP_Zc;
        double *mpU;

    public:
        static Component *Creator()
        {
            return new ElectricVoltageSourceMultiPortC();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("P1", "NodeElectric");
            addInputVariable("U", "Voltage", "V", 12, &mpU);

            disableStartValue(mpMP, NodeElectric::Voltage);
            disableStartValue(mpMP, NodeElectric::WaveVariable);
            disableStartValue(mpMP, NodeElectric::CharImpedance);
        }


        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();
            mvpMP_u.resize(mNumPorts);
            mvpMP_i.resize(mNumPorts);
            mvpMP_c.resize(mNumPorts);
            mvpMP_Zc.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpMP_u[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::Voltage);
                mvpMP_i[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::Current);
                mvpMP_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::WaveVariable);
                mvpMP_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeElectric::CharImpedance);

                *(mvpMP_u[i]) = (*mpU);    //Override the startvalue for the pressure
                *(mvpMP_i[i]) = getDefaultStartValue(mpMP, NodeElectric::Current);
                *(mvpMP_c[i]) = (*mpU);
                *(mvpMP_Zc[i]) = 0.0;
            }
        }


        void simulateOneTimestep()
        {
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *(mvpMP_c[i]) = (*mpU);
                *(mvpMP_Zc[i]) = 0.0;
            }
        }
    };
}

#endif // ELECTRICVOLTAGESOURCEMULTIPORTC_HPP
