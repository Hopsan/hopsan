/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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
//        double tao;

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
            //Assign node data pointers
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

//            double A1 = (*mpA1);
//            double A2 = (*mpA2);
            double bp = (*mpBp);
            double bl = (*mpBl);
            double kl = (*mpKl);

            //Read data from nodes
            double x3 = (*mpP3_x);
            double v3 = (*mpP3_v);
//            double Zc1 = (*mpND_Zc1);
//            double Zc2 = (*mpND_Zc2);
//            double cx3 = (*mpND_cx3);
//            double Zx3 = (*mpND_Zx3);

//            Zx1 = A1*A1*Zc1 + A2*A2*Zc2;
//            tao    = 3.0/2.0*mTimestep;        //Velocity filter time constant, should be in initialize?

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
            //Get variable values from nodes
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


            //CylinderCtest Equations

            //Internal mechanical port
            double cx1 = A1*max(c1,0.) - A2*max(c2,0.);
//            double cx1 = A1*c1 - A2*c2;
            double Zx1 = A1*A1*Zc1 + A2*A2*Zc2;

            //Piston
            mPosDen[1] = bl+bp+Zx1+Zx3;
            mPositionTF.setDen(mPosDen);
            double x3 = mPositionTF.update(cx1-cx3); // (kl is part of denominator)
            double v3;
            if (mPositionTF.isSaturated())
            {
                mVelocityTF.initializeValues(0,0);
                v3 = 0;
            }
            else
            {
                mVelDen[0] = bl+bp+Zx1+Zx3;
                mVelocityTF.setDen(mVelDen);
                v3 = mVelocityTF.update(cx1-cx3 - kl*x3);
            }

            // An alternative way of calculating velocity
            // Note update above shifted one step so delayedY is actually this Y
            //v3 = (mPositionTF.delayedY() - mPositionTF.delayed2Y())/mTimestep;

            double f3 = cx3 + Zx3*v3;

            //Volumes
            double q1 = A1*-v3;
            double q2 = A2*v3;
            double p1 = c1 + Zc1*q1;
            double p2 = c2 + Zc2*q2;

            //Cavitation check
            if(p1 < 0.0) { p1 = 0.0; }
            if(p2 < 0.0) { p2 = 0.0; }

            //Write new values to nodes
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
