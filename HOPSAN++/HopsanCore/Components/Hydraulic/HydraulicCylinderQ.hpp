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

#include <iostream>
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
        double mArea1;
        double mArea2;
        double mStroke;
        double mMass;
        double mBp;
        double mBl;
        double mKl;
        double mTao;
        SecondOrderTransferFunction mPositionFilter;
        FirstOrderTransferFunction mVelocityFilter;
        double posnum[3], posden[3], velnum[3], velden[3];
        double p1, q1, c1, Zc1, p2, q2, c2, Zc2, v1, cx1, Zx1, f3, x3, v3, cx3, Zx3;
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_f3, *mpND_x3, *mpND_v3, *mpND_cx3, *mpND_Zx3;
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderQ();
        }

        HydraulicCylinderQ() : ComponentQ()
        {
            mArea1  = 0.0001;
            mArea2  = 0.0001;
            mStroke = 0.01;
            mMass   = 0.05;
            mBp     = 0.0;
            mBl     = 0;
            mKl     = 1000;
            mTao    = 3.0/2.0*mTimestep;        //Velocity filter time constant, should be in initialize?

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            registerParameter("A_1", "Piston Area 1", "[m^2]", mArea1);
            registerParameter("A_2", "Piston Area 2", "[m^2]", mArea2);
            registerParameter("s_l", "Stroke", "[m]", mStroke);
            registerParameter("B_p", "Viscous Friction Coefficient of Piston", "[Ns/m]", mBp);
            registerParameter("m_l", "Inertia Load", "[kg]", mMass);
            registerParameter("B_l", "Viscous Friction of Load", "[Ns/m]", mBl);
            registerParameter("k_l", "Stiffness of Load", "[N/m]", mKl);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_f3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
            mpND_x3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
            mpND_v3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
            mpND_cx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

            //Read data from nodes
            x3 = (*mpND_x3);
            v3 = (*mpND_v3);
            Zc1 = (*mpND_Zc1);
            Zc2 = (*mpND_Zc2);
            cx3 = (*mpND_cx3);
            Zx3 = (*mpND_Zx3);

            Zx1 = mArea1*mArea1*Zc1 + mArea2*mArea2*Zc2-mBp;

            //Initialization of filters
            posnum[0] = 1.0;
            posnum[1] = 0.0;
            posnum[2] = 0.0;
            posden[0] = mKl;
            posden[1] = mBl;
            posden[2] = mMass;
            velnum[0] = 1.0;
            velnum[1] = 0.0;
            velden[0] = mBl;
            velden[1] = mMass;

            mPositionFilter.initialize(mTimestep, posnum, posden, 0, x3, 0.0, mStroke);
            mVelocityFilter.initialize(mTimestep, velnum, velden, 0, v3);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            cx3 = (*mpND_cx3);
            Zx3 = (*mpND_Zx3);

            //CylinderCtest Equations

            //Internal mechanical port
            cx1 = mArea1*c1 - mArea2*c2;
            Zx1 = mArea1*mArea1*Zc1 + mArea2*mArea2*Zc2;

            //Piston
            posden[1] = mBl+Zx1+Zx3;
            velden[0] = mBl+Zx1+Zx3;
            mPositionFilter.setDen(posden);
            mVelocityFilter.setDen(velden);
            x3 = mPositionFilter.update(cx1-cx3);
            v3 = mVelocityFilter.update(cx1-cx3 - mKl*x3);

            v1 = -v3;
            f3 = cx3 + Zx3*v3;

            //Volumes
            q1 = mArea1*v1;
            q2 = mArea2*v3;
            p1 = c1 + Zc1*q1;
            p2 = c2 + Zc2*q2;

            //Cavitation check
            if(p1 < 0.0) { p1 = 0.0; }
            if(p2 < 0.0) { p2 = 0.0; }

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_f3) = f3;
            (*mpND_x3) = x3;
            (*mpND_v3) = v3;
        }
    };
}

#endif // HYDRAULICCYLINDERQ_HPP_INCLUDED
