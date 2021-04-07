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
//! @file   MechanicCylinderBlockWithSwashPlate.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-12
//!
//! @brief Contains a mechanic cylinder block with swash plate component
//!
//$Id$

#ifndef MECHANICCYLINDERBLOCKWITHSWASHPLATE_HPP_INCLUDED
#define MECHANICCYLINDERBLOCKWITHSWASHPLATE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicCylinderBlockWithSwashPlate : public ComponentQ
    {

    private:
        Port *mpP1, *mpP2;
        size_t mNumPorts1;

        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];

        double *mpAngle, *mpTorque, *mpMovement;
        std::vector<double*> mvpND_f1, mvpND_x1, mvpND_v1, mvpND_c1, mvpND_Zc1, mvpND_me1;
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        double *mpOffset, *mpR, *mpB;

        double t2, a2, w2, c2, Zx2;
        std::vector<double> f1, c1, Zc1, x1,v1;

        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;

        double J, mp, rp, startX;

    public:
        static Component *Creator()
        {
            return new MechanicCylinderBlockWithSwashPlate();
        }

        void configure()
        {
            addInputVariable("B", "Viscous Friction", "Nms/rad", 10.0, &mpB);
            addInputVariable("r", "Swivel Radius", "m", 0.05, &mpR);
            addInputVariable("theta_offset", "Angle Offset", "m", 0.0, &mpOffset);
            addInputVariable("angle", "Angle", "rad", 0, &mpAngle);

            addOutputVariable("torque", "Torque", "Nm", 0.0, &mpTorque);
            addOutputVariable("movement", "?", "?", 0.0, &mpMovement);

            addConstant("J", "Moment of Inertia of Cylinder Block", "MomentOfInertia", 0.1, J);
            addConstant("m_p", "Mass of each Piston", "kg", 0.001, mp);
            addConstant("r_p", "Piston Radius", "m", 0.01, rp);

            mpP1 = addPowerMultiPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
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

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

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

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_me1[i]) = 0.02;
            }

            double r = (*mpR);
            double B = (*mpB);

            startX = (*mvpND_x1[0]);

            t2 = (*mpND_t2);
            a2 = (*mpND_a2);
            w2 = (*mpND_w2);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = 0.0;
            mDenX[1] = B;
            mDenX[2] = J;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = B;
            mDenV[1] = J;

            mFilterX.initialize(mTimestep, mNumX, mDenX, -t2, a2);
            mFilterV.initialize(mTimestep, mNumV, mDenV, -t2, w2);

            //Append moment of inertia from pistons
            J = J + mp*(rp*rp/2 + r*r);
        }


        void simulateOneTimestep()
        {
            double offset = (*mpOffset);
            double r = (*mpR);
            double B = (*mpB);

            //Get variable values from nodes
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Calculate constants
            double angle = (*mpAngle);
            double ta = tan(angle);
            double s = r*ta;
            double diff = 2*pi/double(mNumPorts1);

            //Calculate torque
            double cp = 0;
            double Zp = 0;
            double a1 = -a2;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                c1[i] = (*mvpND_c1[i]);
                Zc1[i] = (*mvpND_Zc1[i]);
                double co = cos(a1-offset-diff*double(i));
                cp += c1[i]*ta*r*co;
                Zp += Zc1[i]*ta*r*co*ta*r*co;
            }

            //Inertia equations
            mDenX[1] = B+Zp+Zx2;
            mDenV[0] = mDenX[1];
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            a2 = mFilterX.update(cp-c2);
            w2 = mFilterV.update(cp-c2);

            t2 = c2 + Zx2*w2;
            double w1 = -w2;

            //Calculate positions and velocities
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                double a = a1-offset-diff*double(i);
                v1[i] = s*cos(a)*w1;
                x1[i] = startX+s*sin(a);
            }

            //Piston forces
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                f1[i] = c1[i]+Zc1[i]*v1[i];
            }

            //Write new values to nodes
            (*mpTorque) = -t2;
            (*mpMovement) = w1;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_f1[i]) = f1[i];
                (*mvpND_x1[i]) = x1[i];
                (*mvpND_v1[i]) = v1[i];
            }
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
        }
    };
}

#endif // MECHANICCYLINDERBLOCKWITHSWASHPLATE_HPP_INCLUDED

