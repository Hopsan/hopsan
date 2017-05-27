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

#ifndef MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicVelocityTransformer : public ComponentQ
    {

    private:
        bool mXIsConnected;
        double me;
        double *mpPm1_f, *mpPm1_x, *mpPm1_v, *mpPm1_c, *mpPm1_Zx, *mpPm1_me;
        double *mpIn_x, *mpIn_v;
        Integrator mInt;
        Port *mpXPort, *mpVPort, *mpPm1;

    public:
        static Component *Creator()
        {
            return new MechanicVelocityTransformer();
        }

        void configure()
        {
            //Add ports to the component
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            mpVPort = addInputVariable("v", "Generated Velocity", "m/s", 0.0, &mpIn_v);
            mpXPort = addInputVariable("x", "Generated Position", "m", 0.0, &mpIn_x);

            // add constants
            addConstant("m_e", "Equivalent Mass", "kg", 10, me);
        }


        void initialize()
        {
            mpPm1_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
            mpPm1_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
            mpPm1_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
            mpPm1_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
            mpPm1_Zx = getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
            mpPm1_me = getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);

            mXIsConnected = mpXPort->isConnected();
            if(mXIsConnected && !mpVPort->isConnected())
            {
                addWarningMessage("Position input is connected but velocity is constant, kinematic relationship must be manually enforced.");
            }
            else if(mXIsConnected && mpVPort->isConnected())
            {
                addWarningMessage("Both position and velocity inputs are connected, kinematic relationship must be manually enforced.");
            }

            // Initialize node values
            mInt.initialize(mTimestep, (*mpIn_v), (*mpIn_x));
            (*mpPm1_me) = me;

            (*mpPm1_f) = (*mpPm1_c) + (*mpPm1_Zx)*(*mpIn_v);
            (*mpPm1_x) = (*mpIn_x);
            (*mpPm1_v) = (*mpIn_v);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            const double v = (*mpIn_v);
            const double c = (*mpPm1_c);
            const double Zx = (*mpPm1_Zx);
            double x;

            //Source equations
            if(mXIsConnected)
            {
                x = (*mpIn_x);
            }
            else
            {
                x = mInt.update(v);
            }

            //Write values to nodes
            (*mpPm1_f) = c + Zx*v;
            (*mpPm1_x) = x;
            (*mpPm1_v) = v;
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
