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
//! @file   MechanicTranslationalSpringWithSlack.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-06-30
//!
//! @brief Contains a mechanical translational spring component with slack
//!
//$Id$

#ifndef MECHANICTRANSLATIONALSPRINGWITHSLACK_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRINGWITHSLACK_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalSpringWithSlack : public ComponentC
    {

    private:
        double *mpK;
        double *mpP1_f, *mpP2_f, *mpP1_x, *mpP1_v, *mpP1_c, *mpP1_Zc, *mpP2_x, *mpP2_v, *mpP2_c, *mpP2_Zc;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalSpringWithSlack();
        }

        void configure()
        {
            // Add power ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            // Add input variables
            addInputVariable("k", "Spring Coefficient", "N/m", 100.0,  &mpK);
        }


        void initialize()
        {
            mpP1_f =  getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpP2_f =  getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            //! @todo Is this correct? Ask Petter!
            //(*mpND_c1) = (*mpND_f2)+2.0*Zc*(*mpND_v2);
            //(*mpND_c2) = (*mpND_f1)+2.0*Zc*(*mpND_v1);
            (*mpP1_Zc) = (*mpK)*mTimestep;
            (*mpP2_Zc) = (*mpK)*mTimestep;
        }


        void simulateOneTimestep()
        {
            double c1,c2,Zc;

            //Get variable values from nodes
            const double x1 = (*mpP1_x);
            const double x2 = (*mpP2_x);
            if(x1+x2 > 0)
            {
                c1 = 0;
                c2 = 0;
                Zc = 0;
            }
            else
            {
                const double v1 = (*mpP1_v);
                const double lastc1 = (*mpP1_c);
                const double v2 = (*mpP2_v);
                const double lastc2 = (*mpP2_c);

                //Spring equations
                Zc = (*mpK)*mTimestep;
                c1 = lastc2 + 2.0*Zc*v2;
                c2 = lastc1 + 2.0*Zc*v1;
            }

            //Write new values to nodes
            (*mpP1_c) = c1;
            (*mpP1_Zc) = Zc;
            (*mpP2_c) = c2;
            (*mpP2_Zc) = Zc;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRINGWITHSLACK_HPP_INCLUDED
