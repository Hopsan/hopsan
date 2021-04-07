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
//! @file   MechanicRackAndPinion.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-06-19
//!
//! @brief Contains a mechanic rack and pinion component
//!
//$Id$

#ifndef MECHANICRACKANDPINION_HPP_INCLUDED
#define MECHANICRACKANDPINION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //Verified with AMESim 2011-03-21
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRackAndPinion : public ComponentQ
    {

    private:
        double *mpGearRatio, *mpJ, *mpB;
        double k;
        double num[3];
        double den[3];
        SecondOrderTransferFunction mFilter;
        Integrator mInt;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_me1, *mpND_c1, *mpND_Zx1,
               *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicRackAndPinion();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            addInputVariable("omega", "Gear ratio", "m/rad", 1.0, &mpGearRatio);
            addInputVariable("J", "Moment of Inertia", "MomentOfInertia", 1.0, &mpJ);
            addInputVariable("B", "Viscous Friction", "Nms/rad", 10, &mpB);

            addConstant("k", "Spring Coefficient", "Nm/rad", 0.0, k);
        }


        void initialize()
        {
            double gearRatio, J, B;
            gearRatio = (*mpGearRatio);
            J = (*mpJ);
            B = (*mpB);

            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            num[0] = 0.0;
            num[1] = 1.0;
            num[2] = 0.0;
            den[0] = k;
            den[1] = B;
            den[2] = J;
            mFilter.initialize(mTimestep, num, den, 0, 0);      //Must initialize with zero, otherwise filter may give static offset with zero input
            mInt.initialize(mTimestep, 0, 0);                   //Must initialize with zero, otherwise filter may give static offset with zero input

            (*mpND_me1) = J*gearRatio;      //! @todo Verifiy that this is correct
        }


        void simulateOneTimestep()
        {
            double f1, x1, v1, c1, Zx1;
            double t2, a2, w2, c2, Zx2;
            double gearRatio, J, B;
            gearRatio = (*mpGearRatio);
            J = (*mpJ);
            B = (*mpB);

            //Get variable values from nodes
            c1  = (*mpND_c1)*gearRatio;
            Zx1 = (*mpND_Zx1)*pow(gearRatio, 2.0);
            c2  = (*mpND_c2);
            Zx2 = (*mpND_Zx2);
            gearRatio = (*mpGearRatio);
            J = (*mpJ);
            B = (*mpB);

            //Mass equations
            den[1] = B+Zx1+Zx2;

            mFilter.setDen(den);
            w2 = mFilter.update(c1-c2);
            v1 = -w2*gearRatio;
            a2 = mInt.update(w2);
            x1 = -a2*gearRatio;
            f1 = (c1 + Zx1*v1)/gearRatio;
            t2 = c2 + Zx2*w2;

            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_me1) = J*gearRatio;      //! @todo Verifiy that this is correct
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
        }
    };
}

#endif // MECHANICRACKANDPINION_HPP_INCLUDED

