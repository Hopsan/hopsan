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
//! @file   MechanicRotationalInertia.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a mechanic rotational inertia component
//!
//$Id$

#ifndef MECHANICROTATIONALINERTIA_HPP_INCLUDED
#define MECHANICROTATIONALINERTIA_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInertia : public ComponentQ
    {

    private:
        double J;
        double *mpB, *mpK, *mpAMin, *mpAMax;
        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];
        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertia();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            addConstant("J", "Moment of Inertia", "MomentOfInertia", 0.1, J);
            addInputVariable("B", "Viscous Friction", "Nms/rad", 10.0, &mpB);
            addInputVariable("k", "Spring Constant", "Nm/rad", 0.0, &mpK);
            addInputVariable("a_min", "Minimum Angle of Port P2", "rad", -1.0e+300, &mpAMin);
            addInputVariable("a_max", "Maximum Angle of Port P2", "rad", 1.0e+300, &mpAMax);
        }


        void initialize()
        {
            mpND_t1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_a1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            double t1, a1, w1, t2, a2, B, k;
            B = (*mpB);
            k = (*mpK);

            t1 = (*mpND_t1);
            t2 = (*mpND_t2);
            a1 = (*mpND_a1);
            a2 = (*mpND_a2);
            w1 = (*mpND_w1);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = k;
            mDenX[1] = B;
            mDenX[2] = J;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = B;
            mDenV[1] = J;

            mFilterX.initialize(mTimestep, mNumX, mDenX, t1-t2, -a1, -1.5e300, 1.5e300, -w1);
            mFilterV.initialize(mTimestep, mNumV, mDenV, t1-t2 - k*a2, -w1);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double B, k, t1, a1, w1, c1, Zx1, t2, a2, w2, c2, Zx2;
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);
            B = (*mpB);
            k = (*mpK);

            //Inertia equations
            mDenX[1] = B+Zx1+Zx2;
            mDenV[0] = B+Zx1+Zx2;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            a2 = mFilterX.update(c1-c2);
            w2 = mFilterV.update(c1-c2 - k*a2);

            if(a2<(*mpAMin))
            {
                a2=(*mpAMin);
                w2=0.0;
                mFilterX.initializeValues(c1-c2, a2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }
            if(a2>(*mpAMax))
            {
                a2=(*mpAMax);
                w2=0.0;
                mFilterX.initializeValues(c1-c2, a2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }

            w1 = -w2;
            a1 = -a2;
            t1 = c1 + Zx1*w1;
            t2 = c2 + Zx2*w2;

            //Write new values to nodes
            (*mpND_t1) = t1;
            (*mpND_a1) = a1;
            (*mpND_w1) = w1;
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
        }
    };
}

#endif // MECHANICROTATIONALINERTIA_HPP_INCLUDED

