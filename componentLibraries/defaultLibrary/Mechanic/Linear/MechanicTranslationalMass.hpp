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

#ifndef MECHANICTRANSLATIONALMASS_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASS_HPP_INCLUDED

#include <math.h>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMass : public ComponentQ
    {

    private:
        // Port pointers
        Port *mpP1, *mpP2;
        // Node data pointers
        double *mpP1_f, *mpP1_x, *mpP1_v, *mpP1_c, *mpP1_Zx, *mpP1_me;
        double *mpP2_f, *mpP2_x, *mpP2_v, *mpP2_c, *mpP2_Zx, *mpP2_me;
        double *mpB, *mpK, *mpXMin, *mpXMax;
        // Constants
        double mMass;

        // Other member variables
        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];

        // This length is not accessible by the user,
        // it is set from the start values by the c-components in the ends
        double mLength;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMass();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            addConstant("m", "Mass", "kg",                      100.0, mMass);
            addInputVariable("B", "Viscous Friction", "Ns/m",   10.0, &mpB);
            addInputVariable("k", "Spring Coefficient", "N/m", 0.0, &mpK);
            addInputVariable("x_min", "Minimum Position of Port P2", "m", -1.0e+300, &mpXMin);
            addInputVariable("x_max", "Maximum Position of Port P2", "m", 1.0e+300, &mpXMax);
        }


        void initialize()
        {
            //Assign node data pointers
            mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpP1_me = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);

            mpP2_f = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);
            mpP2_me = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);

            //Initialization
            double f1, x1, /*v1,*/ f2, x2, v2;
            f1 = (*mpP1_f);
            f2 = (*mpP2_f);
            x1 = (*mpP1_x);
            //v1 = (*mpP1_v);
            x2 = (*mpP2_x);
            v2 = (*mpP2_v);

            mLength = x1+x2;

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = (*mpK);
            mDenX[1] = (*mpB);
            mDenX[2] = mMass;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = (*mpB);
            mDenV[1] = mMass;

            mFilterX.initialize(mTimestep, mNumX, mDenX, f1-f2, x2, -1.5e300, 1.5e300, v2);
            mFilterV.initialize(mTimestep, mNumV, mDenV, f1-f2 - (*mpK)*x2, v2);

            (*mpP1_me) = mMass;
            (*mpP2_me) = mMass;

            //Print debug message if velocities do not match
            if((*mpP1_v) != -(*mpP2_v))
            {
                this->addErrorMessage("Start velocities does not match, {"+getName()+"::"+mpP1->getName()+
                                      "} and {"+getName()+"::"+mpP2->getName()+"}.");
                stopSimulation();
            }
        }


        void simulateOneTimestep()
        {
            double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;

            //Get variable values from nodes
            c1 = (*mpP1_c);
            Zx1 = (*mpP1_Zx);
            c2 = (*mpP2_c);
            Zx2 = (*mpP2_Zx);
            const double k = (*mpK);
            const double B = (*mpB);

            //Mass equations
            mDenX[0] = k;
            mDenX[1] = B+Zx1+Zx2;
            mDenV[0] = B+Zx1+Zx2;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            x2 = mFilterX.update(c1-c2);
            v2 = mFilterV.update(c1-c2 - k*x2);

            if(x2<(*mpXMin))
            {
                x2=(*mpXMin);
                v2=0.0;
                mFilterX.initializeValues(c1-c2, x2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }
            if(x2>(*mpXMax))
            {
                x2=(*mpXMax);
                v2=0.0;
                mFilterX.initializeValues(c1-c2, x2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }

            v1 = -v2;
            x1 = -x2 + mLength;
            f1 = c1 + Zx1*v1;
            f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*mpP1_f) = f1;
            (*mpP1_x) = x1;
            (*mpP1_v) = v1;
            (*mpP2_f) = f2;
            (*mpP2_x) = x2;
            (*mpP2_v) = v2;
            (*mpP1_me) = mMass;
            (*mpP2_me) = mMass;
        }
    };
}

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

