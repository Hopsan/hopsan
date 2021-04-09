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
//! @file   MechanicSwashPlate.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-12
//!
//! @brief Contains a mechanic swivel plate component
//!
//$Id$

#ifndef MECHANICSWASHPLATE_HPP_INCLUDED
#define MECHANICSWASHPLATE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicSwashPlate : public ComponentQ
    {

    private:
        Port *mpP1;
        double *mpAngle, *mpMovement, *mpTorque;
        std::vector<double*> mvpND_f1, mvpND_x1, mvpND_v1, mvpND_c1, mvpND_Zc1, mvpND_me1;
        size_t mNumPorts1;

        double *mpR, *mpOffset;

        Integrator mIntegrator;

        double startX;

        std::vector<double> f1, c1, Zc1, x1,v1;

    public:
        static Component *Creator()
        {
            return new MechanicSwashPlate();
        }

        void configure()
        {
            //Register changeable parameters to the HOPSAN++ core
            addInputVariable("r", "Swivel Radius", "m", 0.05, &mpR);
            addInputVariable("theta_offset", "Angle Offset", "m", 0.0, &mpOffset);
            addInputVariable("angle", "Angle", "rad", 0, &mpAngle);
            addInputVariable("movement", "Angular velocity", "rad/s", 0, &mpMovement);
            addOutputVariable("torque", "Torque", "Nm", &mpTorque);

            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeMechanic");
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mvpND_f1.resize(mNumPorts1);
            mvpND_x1.resize(mNumPorts1);
            mvpND_v1.resize(mNumPorts1);
            mvpND_c1.resize(mNumPorts1);
            mvpND_Zc1.resize(mNumPorts1);
            mvpND_me1.resize(mNumPorts1);
            f1.resize(mNumPorts1);
            c1.resize(mNumPorts1);
            Zc1.resize(mNumPorts1);
            x1.resize(mNumPorts1);
            v1.resize(mNumPorts1);

            //Assign node data pointers
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                mvpND_f1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Force, 0.0);
                mvpND_x1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Position, 0.0);
                mvpND_v1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Velocity, 0.0);
                mvpND_c1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::WaveVariable, 0.0);
                mvpND_Zc1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::CharImpedance, 0.0);
                mvpND_me1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::EquivalentMass, 0.0);
            }

            mIntegrator.initialize(mTimestep, 0.0, 0.0);

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_me1[i]) = 0.02;
            }

            startX = (*mvpND_x1[0]);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double angle = (*mpAngle);
            double w1 = (*mpMovement);
            double r = (*mpR);
            double offset = (*mpOffset);

            double s = r*tan(angle);
            double diff = 2*pi/double(mNumPorts1);

            double a1 = mIntegrator.update(w1);

            //Calculate positions and velocities
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                v1[i] = s*cos(a1-offset-diff*double(i))*w1;
                x1[i] = startX+s*sin(a1-offset-diff*double(i));
            }

            //Piston forces
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                c1[i] = (*mvpND_c1[i]);
                Zc1[i] = (*mvpND_Zc1[i]);
                f1[i] = c1[i]+Zc1[i]*v1[i];
            }

            //Calculate torque
            double torque = 0;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                torque += f1[i]*tan(angle)*r*cos(a1-offset-diff*double(i));
            }

            //Write new values to nodes
            (*mpTorque) = torque;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_f1[i]) = f1[i];
                (*mvpND_x1[i]) = x1[i];
                (*mvpND_v1[i]) = v1[i];
            }
        }
    };
}

#endif // MECHANICSWASHPLATE_HPP_INCLUDED

