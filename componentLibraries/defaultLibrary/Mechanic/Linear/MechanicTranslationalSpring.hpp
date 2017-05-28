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

#ifndef MECHANICTRANSLATIONALSPRING_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRING_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalSpring : public ComponentC
    {

    private:
        double *mpK;
        double *mpP1_f, *mpP2_f, *mpP1_v, *mpP1_c, *mpP1_Zc, *mpP2_v, *mpP2_c, *mpP2_Zc;
        double mAlpha;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalSpring();
        }

        void configure()
        {
            // Add power ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            // Add input variables
            addInputVariable("k", "Spring Coefficient", "N/m", 100.0,  &mpK);

            addConstant("alpha", "Euler-fwd TLM filter, (0<=a<1)", "", 0, mAlpha);
        }


        void initialize()
        {
            mpP1_f =  getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP2_f =  getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            const double Zc = (*mpK)*mTimestep/(1.0-mAlpha);
            (*mpP1_c) = (*mpP2_f)+Zc*(*mpP2_v);
            (*mpP2_c) = (*mpP1_f)+Zc*(*mpP1_v);
            (*mpP1_Zc) = Zc;
            (*mpP2_Zc) = Zc;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            const double v1 = (*mpP1_v);
            const double lastc1 = (*mpP1_c);
            const double v2 = (*mpP2_v);
            const double lastc2 = (*mpP2_c);

            //Spring equations
            const double Zc = (*mpK)*mTimestep/(1.0-mAlpha);
            const double c1 = (lastc2 + 2.0*Zc*v2)*(1.0-mAlpha)+lastc1*mAlpha ;
            const double c2 = (lastc1 + 2.0*Zc*v1)*(1.0-mAlpha)+lastc2*mAlpha;

            //Write new values to nodes
            (*mpP1_c) = c1;
            (*mpP1_Zc) = Zc;
            (*mpP2_c) = c2;
            (*mpP2_Zc) = Zc;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


