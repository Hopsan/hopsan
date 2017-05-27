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

#ifndef MECHANICTRANSLATIONALMASSVARIABLE_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASSVARIABLE_HPP_INCLUDED

#include <math.h>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassVariable : public ComponentQ
    {

    private:
        double m;
        double *mpB, *mpK, *xMin, *xMax;
        double mLength;         //This length is not accessible by the user,
                                //it is set from the start values by the c-components in the ends
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_me1, *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2, *mpND_me2;  //Node data pointers
        double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;                                                    //Node data variables
        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];
        SecondOrderTransferFunctionVariable mFilterX;
        FirstOrderTransferFunctionVariable mFilterV;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassVariable();
        }

        void configure()
        {
            //mCanHandleVariableTimeStep = true;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            addConstant("m", "Mass", "kg",                      100.0, m);
            addInputVariable("B", "Viscous Friction", "Ns/m",   10.0, &mpB);
            addInputVariable("k", "Spring Coefficient", "N/m", 0.0, &mpK);
            addInputVariable("x_min", "Minimum Position", "m", 0.0, &xMin);
            addInputVariable("x_max", "Maximum Position", "m", 1.0, &xMax);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);
            mpND_me2 = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);

            //Initialization
            f1 = (*mpND_f1);
            f2 = (*mpND_f2);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            x2 = (*mpND_x2);
            v2 = (*mpND_v2);

            mLength = x1+x2;

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = (*mpK);
            mDenX[1] = (*mpB);
            mDenX[2] = m;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = (*mpB);
            mDenV[1] = m;

            mFilterX.initialize(&mTimestep, mNumX, mDenX, f1-f2, x2);
            mFilterV.initialize(&mTimestep, mNumV, mDenV, f1-f2 - (*mpK)*x2, v2);

            (*mpND_me1) = m;
            (*mpND_me2) = m;

            //Print debug message if velocities do not match
            if((*mpND_v1) != -(*mpND_v2))
            {
                this->addErrorMessage("Start velocities does not match, {"+getName()+"::"+mpP1->getName()+
                                      "} and {"+getName()+"::"+mpP2->getName()+"}.");
                stopSimulation();
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);
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

//            if((mTime > 6) && (mTime < 6.01))
//            {
//                double apa = c1-c2;
//                stringstream ss;
//                ss << "t: " << mTime << "   c1-c2 = " << apa << "   v2 = " << v2;
//                addDebugMessage(ss.str());
//            }

            if(x2<(*xMin))
            {
                x2=(*xMin);
                v2=0.0;
                mFilterX.initializeValues(c1-c2, x2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }
            if(x2>(*xMax))
            {
                x2=(*xMax);
                v2=0.0;
                mFilterX.initializeValues(c1-c2, x2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }

            v1 = -v2;
            x1 = -x2 + mLength;
            f1 = c1 + Zx1*v1;
            f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
            (*mpND_me1) = m;
            (*mpND_me2) = m;


//            if((mTime>.5) && (mTime<.5001))
//            {
//                std::stringstream ss;
//                ss << "mTime: " << mTime << "     c1-c2: " << c1-c2 << "    v2: " << v2 << std::endl;
//                addInfoMessage(ss.str());
//            }
        }
    };
}

#endif // MECHANICTRANSLATIONALMASSVARIABLE_HPP_INCLUDED

