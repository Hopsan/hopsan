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
//! @file   MechanicRotationalInertiaWithSingleGear.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-03-21
//!
//! @brief Contains a mechanic rotational gear ratio with inertia component
//!
//$Id$

#ifndef MECHANICROTATIONALINERTIAWITHSINGLEGEAR_HPP_INCLUDED
#define MECHANICROTATIONALINERTIAWITHSINGLEGEAR_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //! @details
    //! Implements the following motion equations
    //! \f{eqnarray*}{
    //! J\dot{\omega}_{2} + B\omega_{2} &=& U_{n}T_{1}-T_{2} \\
    //! \omega_{1} &=& -U_{n}\omega_{2} \\
    //! \theta_{1} &=& -U_{n}\theta_{2} \\
    //! U_{n} &=& -U
    //! \f}
    //!
    class MechanicRotationalInertiaWithSingleGear : public ComponentQ
    {

    private:
        double *mpGearRatio, *mpB;
        double J;
        double mNumTheta[3];
        double mDenTheta[3];
        double mNumOmega[2];
        double mDenOmega[2];
        SecondOrderTransferFunction mFilterTheta;
        FirstOrderTransferFunction mFilterOmega;
        double *mpP1_t, *mpP1_a, *mpP1_w, *mpP1_c, *mpP1_Zx,
               *mpP2_t, *mpP2_a, *mpP2_w, *mpP2_c, *mpP2_Zx;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertiaWithSingleGear();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            addInputVariable("omega", "Gear ratio", "-", 1.0, &mpGearRatio);
            addInputVariable("B", "Viscous Friction", "Nms/rad", 10.0, &mpB);
            addConstant("J", "Moment of Inertia", "MomentOfInertia", 0.1, J);
        }


        void initialize()
        {
            mpP1_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpP1_a = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpP1_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

            mpP2_t = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpP2_a = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpP2_w = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            double t1, t2, a2, w2, U, B;
            U = -(*mpGearRatio);
            B = (*mpB);
            t1 = (*mpP1_t);
            t2 = (*mpP2_t);
            a2 = (*mpP2_a);
            w2 = (*mpP2_w);

            mNumTheta[0] = 1.0;
            mNumTheta[1] = 0.0;
            mNumTheta[2] = 0.0;
            mDenTheta[0] = 0;
            mDenTheta[1] = B;
            mDenTheta[2] = J;
            mNumOmega[0] = 1.0;
            mNumOmega[1] = 0.0;
            mDenOmega[0] = B;
            mDenOmega[1] = J;

            mFilterTheta.initialize(mTimestep, mNumTheta, mDenTheta, U*t1-t2, a2, -1.5e300, 1.5e300, w2);
            mFilterOmega.initialize(mTimestep, mNumOmega, mDenOmega, U*t1-t2, w2);
        }

        void simulateOneTimestep()
        {
            double t1, a1, w1, c1, Zx1, t2, a2, w2, c2, Zx2, U, B;

            //Get variable values from nodes
            U = -(*mpGearRatio);
            B = (*mpB);
            c1  = (*mpP1_c);
            Zx1 = (*mpP1_Zx);
            c2  = (*mpP2_c);
            Zx2 = (*mpP2_Zx);

            //Mass equations
            const double BB = B+U*U*Zx1+Zx2;
            mDenTheta[1] = BB;
            mDenOmega[0] = BB;
            mFilterTheta.setDen(mDenTheta);
            mFilterOmega.setDen(mDenOmega);

            const double TT = U*c1-c2;
            a2 = mFilterTheta.update(TT);
            w2 = mFilterOmega.update(TT);
            t2 = c2 + Zx2*w2;

            w1 = -w2*U;
            a1 = -a2*U;
            t1 = c1 + Zx1*w1;

            //Write new values to nodes
            (*mpP1_t) = t1;
            (*mpP1_a) = a1;
            (*mpP1_w) = w1;
            (*mpP2_t) = t2;
            (*mpP2_a) = a2;
            (*mpP2_w) = w2;
        }
    };
}

#endif // MECHANICROTATIONALINERTIAWITHSINGLEGEAR_HPP_INCLUDED

