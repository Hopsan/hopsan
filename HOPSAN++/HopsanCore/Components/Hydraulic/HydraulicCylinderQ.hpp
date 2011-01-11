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
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

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
        SecondOrderFilter mPositionFilter;
        SecondOrderFilter mVelocityFilter;
        double posnum[3], posden[3], velnum[3], velden[3];
        double p1, q1, c1, Zc1, p2, q2, c2, Zc2, v1, cx1, Zx1, f2, x2, v2, cx2, Zx2;
        double *p1_ptr, *q1_ptr, *c1_ptr, *Zc1_ptr, *p2_ptr, *q2_ptr, *c2_ptr, *Zc2_ptr, *f2_ptr, *x2_ptr, *v2_ptr, *cx2_ptr, *Zx2_ptr;
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderQ("CylinderQ");
        }

        HydraulicCylinderQ(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicCylinderQ";
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

            registerParameter("Area1", "Piston Area 1", "m^2", mArea1);
            registerParameter("Area2", "Piston Area 2", "m^2", mArea2);
            registerParameter("Stroke", "Stroke", "m", mStroke);
            registerParameter("Bp", "Viscous Friction Coefficient of Piston", "Ns/m", mBp);
            registerParameter("Mass", "Inertia Load", "kg", mMass);
            registerParameter("Bl", "Viscous Friction of Load", "Ns/m", mBl);
            registerParameter("Kl", "Stiffness of Load", "N/m", mKl);
        }


        void initialize()
        {
            //Assign node data pointers
            p1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            c1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);
            p2_ptr = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q2_ptr = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            c2_ptr = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc2_ptr = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);
            f2_ptr = mpP1->getNodeDataPtr(NodeMechanic::FORCE);
            x2_ptr = mpP1->getNodeDataPtr(NodeMechanic::POSITION);
            v2_ptr = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            cx2_ptr = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2_ptr = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Read data from nodes
            x2 = (*x2_ptr);
            v2 = (*v2_ptr);
            Zc1 = (*Zc1_ptr);
            Zc2 = (*Zc2_ptr);
            cx2 = (*cx2_ptr);
            Zx2 = (*Zx2_ptr);

            Zx1 = mArea1*mArea1*Zc1 + mArea2*mArea2*Zc2-mBp;

            //Initialization of filters
            posnum[0] = 0.0;
            posnum[1] = 0.0;
            posnum[2] = 1.0;
            posden[0] = mMass;
            posden[1] = mBl+Zx1+Zx2;
            posden[2] = mKl;
            velnum[0] = 0.0;
            velnum[1] = 1.0;
            velnum[2] = 0.0;
            velden[0] = 0.0;
            velden[1] = mTao;
            velden[2] = 1.0;

            mPositionFilter.initialize(mTimestep, posnum, posden, cx2, x2, 0.0, mStroke);
            mVelocityFilter.initialize(mTimestep, velnum, velden, x2, v2);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*c1_ptr);
            Zc1 = (*Zc1_ptr);
            c2 = (*c2_ptr);
            Zc2 = (*Zc2_ptr);
            cx2 = (*cx2_ptr);
            Zx2 = (*Zx2_ptr);

            //CylinderCtest Equations

            //Internal mechanical port
            cx1 = mArea1*c1 - mArea2*c2;
            Zx1 = mArea1*mArea1*Zc1 + mArea2*mArea2*Zc2-mBp;

            //Piston
            posden [1] = mBl+Zx1+Zx2;
            mPositionFilter.setNumDen(posnum, posden);
            mPositionFilter.update(cx1-cx2);
            x2 = mPositionFilter.value();

            mVelocityFilter.update(x2);
            v2 = mVelocityFilter.value();

            v1 = -v2;
            f2 = cx2 + Zc2*v2;

            //Volumes
            q1 = mArea1*v1;
            q2 = mArea2*v2;
            p1 = c1 + Zc1*q1;
            p2 = c2 + Zc2*q2;

            //Write new values to nodes
            (*p1_ptr) = p1;
            (*q1_ptr) = q1;
            (*p2_ptr) = p2;
            (*q2_ptr) = q2;
            (*f2_ptr) = f2;
            (*x2_ptr) = x2;
            (*v2_ptr) = v2;
        }
    };
}

#endif // HYDRAULICCYLINDERQ_HPP_INCLUDED
