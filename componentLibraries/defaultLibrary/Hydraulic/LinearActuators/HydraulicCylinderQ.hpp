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
//! @file   HydraulicCylinderQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Hydraulic Cylinder of Q type with mass load
//!
//$Id$

////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//   <--------------Stroke--------------->                                        //
//                                                                                //
//   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                XXXXXXXX                //
//   X       Area1  X  X  Area2          X               X        X               //
//   X              X  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        X               //
//   X x1,v1,f1 <---O  X                                 X  Mass  O---> x2,v2,f2  //
//   X              X  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        X               //
//   X              X  X                 X               X        X               //
//   XXOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXOXX                XXXXXXXX                //
//     |                               |                                          //
//     |                               |                                          //
//     V p1,q1                         V p2,q2                                    //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

#ifndef HYDRAULICCYLINDERQ_HPP_INCLUDED
#define HYDRAULICCYLINDERQ_HPP_INCLUDED

//#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicCylinderQ : public ComponentQ
    {
    private:
        // Members
        SecondOrderTransferFunction mPositionTF;
        FirstOrderTransferFunction mVelocityTF;
        double mPosNum[3], mPosDen[3], mVelNum[3], mVelDen[3];

        // Constants
        double mMass, mSl;

        // Ports and node data pointers
        Port *mpP1, *mpP2, *mpP3;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc, *mpP3_f, *mpP3_x, *mpP3_v, *mpP3_c, *mpP3_Zx;
        double *mpA1, *mpA2, *mpBp, *mpBl, *mpKl;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            addInputVariable("A_1", "Piston Area 1", "m^2", 0.0001, &mpA1);
            addInputVariable("A_2", "Piston Area 2", "m^2", 0.0001, &mpA2);
            addInputVariable("B_p", "Viscous Friction Coefficient of Piston", "Ns/m", 0.0, &mpBp);
            addInputVariable("B_l", "Viscous Friction of Load", "Ns/m", 0.0, &mpBl);
            addInputVariable("k_l", "Stiffness of Load", "N/m", 100, &mpKl);

            addConstant("m_l", "Inertia Load", "kg", 0.05, mMass);
            addConstant("s_l", "Stroke", "m", 0.01, mSl);
        }


        void initialize()
        {
            // Assign node data pointers
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
            mpP3_f = getSafeNodeDataPtr(mpP3, NodeMechanic::Force);
            mpP3_x = getSafeNodeDataPtr(mpP3, NodeMechanic::Position);
            mpP3_v = getSafeNodeDataPtr(mpP3, NodeMechanic::Velocity);
            mpP3_c = getSafeNodeDataPtr(mpP3, NodeMechanic::WaveVariable);
            mpP3_Zx = getSafeNodeDataPtr(mpP3, NodeMechanic::CharImpedance);

            double bp = (*mpBp);
            double bl = (*mpBl);
            double kl = (*mpKl);

            //Read data from nodes
            double x3 = (*mpP3_x);
            double v3 = (*mpP3_v);

            // Initialization of filters
            mPosNum[0] = 1.0;
            mPosNum[1] = 0.0;
            mPosNum[2] = 0.0;
            mPosDen[0] = kl;
            mPosDen[1] = bl+bp;
            mPosDen[2] = mMass;
            mVelNum[0] = 1.0;
            mVelNum[1] = 0.0;
            mVelDen[0] = bl+bp;
            mVelDen[1] = mMass;

            mPositionTF.initialize(mTimestep, mPosNum, mPosDen, 0, x3, 0.0, mSl);
            mVelocityTF.initialize(mTimestep, mVelNum, mVelDen, 0, v3);
        }


        void simulateOneTimestep()
        {
            // Get variable values from nodes
            double c1 = (*mpP1_c);
            double Zc1 = (*mpP1_Zc);
            double c2 = (*mpP2_c);
            double Zc2 = (*mpP2_Zc);
            double cx3 = (*mpP3_c);
            double Zx3 = (*mpP3_Zx);

            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double bp = (*mpBp);
            double bl = (*mpBl);
            double kl = (*mpKl);

            double p1,q1,p2,q2,x3,v3,f3;

            bool doRecalc;
            bool cavP1 = false, cavP2 = false;
            // Recalculation loop in case we get cavitation in a chamber
            do
            {
                doRecalc=false;

                // First we transform the hydraulic wave variables into the mechanic domain
                // along the piston axis by multiplying with the area (pressure*area = force)
                // We also express the CharacteristicImedance as a mechanic imadance for the piston
                // Flow * Zc = Presssure
                // Pressure * Area = Force
                // Speed * Area = Flow
                // Flow * Zc * Area = Force and so Speed * Area * Zc * Area = Force
                // So: CharImp. in the mechanical domain is Zx = A*A*Zc

                // We introduce the internal mechanical port x1, and convert c and Zc into cx1 and Zx1
                // Positive force direction towards port 2
                double cx1 = A1*c1 - A2*c2;
                double Zx1 = A1*A1*Zc1 + A2*A2*Zc2;

                // Piston, mass modelled as a transfere function, damping modified according to internal end external mechanix char imp
                mPosDen[1] = bl+bp+Zx1+Zx3;
                mPositionTF.setDen(mPosDen);
                x3 = mPositionTF.updateWithBackup(cx1-cx3); // (kl is part of denominator so not needed as input to TF)

                // If position is saturated then set velocity zero and reinitialize the velocity TF
                if (mPositionTF.isSaturated())
                {
                    mVelocityTF.backup();
                    mVelocityTF.initializeValues(0,0);
                    v3 = 0;
                }
                else
                {
                    mVelDen[0] = bl+bp+Zx1+Zx3;
                    mVelocityTF.setDen(mVelDen);
                    v3 = mVelocityTF.updateWithBackup(cx1-cx3 - kl*x3);
                }

                // An alternative way of calculating velocity
                // Note update above shifted one step so delayedY is actually this Y
                //v3 = (mPositionTF.delayedY() - mPositionTF.delayed2Y())/mTimestep;

                // Now run the typicla Q-component equations to detemrine the intensity and flow variables p,q,v,f
                // Mechanic force
                f3 = cx3 + Zx3*v3;

                // Hydraulic pressure and flow
                q1 = A1*-v3;
                q2 = A2*v3;
                p1 = c1 + Zc1*q1;
                p2 = c2 + Zc2*q2;

                // Cavitation check
                if(p1 < 0.0)
                {
                    p1 = 0.0;
                    c1 = 0.0;
                    Zc1 = 0.0;
                    // Prevent recalculating again if p1<0 second time
                    if (!cavP1)
                    {
                        cavP1=true;
                        doRecalc=true;
                    }
                }
                if(p2 < 0.0)
                {
                    p2 = 0.0;
                    c2 = 0.0;
                    Zc2 = 0.0;
                    // Prevent recalculating again if p2<0 second time
                    if (!cavP2)
                    {
                        cavP1=true;
                        doRecalc=true;
                    }
                }
                if (doRecalc)
                {
                    mPositionTF.restoreBackup();
                    mVelocityTF.restoreBackup();
                }
            }while(doRecalc);

            // Write new values to nodes
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpP3_f) = f3;
            (*mpP3_x) = x3;
            (*mpP3_v) = v3;
        }
    };
}

#endif // HYDRAULICCYLINDERQ_HPP_INCLUDED
