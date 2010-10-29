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
            //Initialization of filters

            double x2 = mpP3->readNode(NodeMechanic::POSITION);
            double v2 = mpP3->readNode(NodeMechanic::VELOCITY);
            //double c1 = mPortPtrs[P1]->readNode(NodeMechanic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeMechanic::CHARIMP);
            //double c2 = mPortPtrs[P2]->readNode(NodeMechanic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeMechanic::CHARIMP);
            //double cx1 = mArea1*c1 - mArea2*c2;
            double Zx1 = mArea1*mArea1*Zc1 + mArea2*mArea2*Zc2-mBp;
            double cx2 = mpP3->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx2 = mpP3->readNode(NodeMechanic::CHARIMP);

            double posnum [3] = {0.0, 0.0, 1.0};
            double posden [3] = {mMass, mBl+Zx1+Zx2, mKl};
            double velnum [3] = {0.0, 1.0, 0.0};
            double velden [3] = {0.0, mTao, 1.0};
            mPositionFilter.initialize(mTimestep, posnum, posden, cx2, x2, 0.0, mStroke);
            mVelocityFilter.initialize(mTimestep, velnum, velden, x2, v2);

            //mPositionFilter.update(cx1-cx2);
            //mVelocityFilter.update(cx1-cx2);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
            double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);
            double cx2 = mpP3->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zx2 = mpP3->readNode(NodeHydraulic::CHARIMP);

            //CylinderCtest Equations

            //Internal mechanical port
            double cx1 = mArea1*c1 - mArea2*c2;
            double Zx1 = mArea1*mArea1*Zc1 + mArea2*mArea2*Zc2-mBp;

            //Piston
            double posnum [3] = {0.0, 0.0, 1.0};
            double posden [3] = {mMass, mBl+Zx1+Zx2, mKl};
            mPositionFilter.setNumDen(posnum, posden);
            mPositionFilter.update(cx1-cx2);
            double x2 = mPositionFilter.value();

            double velnum [3] = {0.0, 1.0, 0.0};
            double velden [3] = {0.0, mTao, 1.0};
            mVelocityFilter.setNumDen(velnum, velden);
            mVelocityFilter.update(x2);
            double v2 = mVelocityFilter.value();

            double v1 = -v2;
            double F2 = cx2 + Zc2*v2;

            //Volumes
            double q1 = mArea1*v1;
            double q2 = mArea2*v2;
            double p1 = c1 + Zc1*q1;
            double p2 = c2 + Zc2*q2;

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
            mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
            mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
            mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);
            mpP3->writeNode(NodeMechanic::POSITION, x2);
            mpP3->writeNode(NodeMechanic::VELOCITY, v2);
            mpP3->writeNode(NodeMechanic::FORCE, F2);
        }
    };


    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOptimizedCylinderQ : public ComponentQ
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
        Port *mpP1, *mpP2, *mpP3;

        double *c1, *Zc1, *c2, *Zc2, *cx2, *Zx2;
        double *p1, *q1, *p2, *q2, *x2, *v2, *F2;
        double cx1, Zx1, posnum[3], posden[3], velnum[3], velden[3], v1;

            //Filter
        double mPosFilterDelayU[2];
        double mPosFilterDelayY[2];
        double mPosFilterCoeffU[3];
        double mPosFilterCoeffY[3];

        double mVelFilterDelayU[2];
        double mVelFilterDelayY[2];
        double mVelFilterCoeffU[3];
        double mVelFilterCoeffY[3];
            //
        //SecondOrderFilter mPositionFilter;

    public:
        static Component *Creator()
        {
            return new HydraulicOptimizedCylinderQ("CylinderQ");
        }

        HydraulicOptimizedCylinderQ(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicOptimizedCylinderQ";
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
            //C pointers
            c1 = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc1 = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);
            c2 = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc2 = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);
            cx2 = mpP3->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2 = mpP3->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Q pointers
            p1 = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q1 = mpP1->getNodeDataPtr(NodeHydraulic::MASSFLOW);
            p2 = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q2 = mpP2->getNodeDataPtr(NodeHydraulic::MASSFLOW);
            x2 = mpP3->getNodeDataPtr(NodeMechanic::POSITION);
            v2 = mpP3->getNodeDataPtr(NodeMechanic::VELOCITY);
            F2 = mpP3->getNodeDataPtr(NodeMechanic::FORCE);


            double Zx1 = mArea1*mArea1 * *Zc1 + mArea2*mArea2 * *Zc2 - mBp;

            posnum[0] = 0.0;
            posnum[1] = 0.0;
            posnum[2] = 1.0;
            posden[0] = mMass;
            posden[1] = mBl + Zx1 + *Zx2;
            posden[2] = mKl;
            velnum[0] = 0.0;
            velnum[1] = 1.0;
            velnum[2] = 0.0;
            velden[0] = 0.0;
            velden[1] = mTao;
            velden[2] = 1.0;

                //Filter
            mPosFilterDelayU[0] = *cx2;
            mPosFilterDelayU[1] = *cx2;
            mPosFilterDelayY[0] = *x2;
            mPosFilterDelayY[1] = *x2;

            mPosFilterCoeffU[0] = posnum[2]*(mTimestep*mTimestep) - 2.0*posnum[1]*mTimestep + 4.0*posnum[0];
            mPosFilterCoeffU[1] = 2.0*posnum[2]*(mTimestep*mTimestep) - 8.0*posnum[0];
            mPosFilterCoeffU[2] = posnum[2]*(mTimestep*mTimestep) + 2.0*posnum[1]*mTimestep + 4.0*posnum[0];

            mPosFilterCoeffY[0] = posden[2]*(mTimestep*mTimestep) - 2.0*posden[1]*mTimestep + 4.0*posden[0];
            mPosFilterCoeffY[1] = 2.0*posden[2]*(mTimestep*mTimestep) - 8.0*posden[0];
            mPosFilterCoeffY[2] = posden[2]*(mTimestep*mTimestep) + 2.0*posden[1]*mTimestep + 4.0*posden[0];

            mVelFilterDelayU[0] = *x2;
            mVelFilterDelayU[1] = *x2;
            mVelFilterDelayY[0] = *v2;
            mVelFilterDelayY[1] = *v2;

            mVelFilterCoeffU[0] = velnum[2]*(mTimestep*mTimestep) - 2.0*velnum[1]*mTimestep + 4.0*velnum[0];
            mVelFilterCoeffU[1] = 2.0*velnum[2]*(mTimestep*mTimestep) - 8.0*velnum[0];
            mVelFilterCoeffU[2] = velnum[2]*(mTimestep*mTimestep) + 2.0*velnum[1]*mTimestep + 4.0*velnum[0];

            mVelFilterCoeffY[0] = velden[2]*(mTimestep*mTimestep) - 2.0*velden[1]*mTimestep + 4.0*velden[0];
            mVelFilterCoeffY[1] = 2.0*velden[2]*(mTimestep*mTimestep) - 8.0*velden[0];
            mVelFilterCoeffY[2] = velden[2]*(mTimestep*mTimestep) + 2.0*velden[1]*mTimestep + 4.0*velden[0];
                //

              //mVelocityFilter.initialize(mTime, mTimestep, velnum, velden, *x2, *v2);
        }


        void simulateOneTimestep()
        {


            //CylinderCtest Equations

            //Internal mechanical port
            cx1 = mArea1 * *c1 - mArea2 * *c2;
            Zx1 = mArea1*mArea1 * *Zc1 + mArea2*mArea2 * *Zc2 - mBp;

            //Piston
            posnum[0] = 0.0;
            posnum[1] = 0.0;
            posnum[2] = 1.0;
            posden[0] = mMass;
            posden[1] = mBl + Zx1 + *Zx2;
            posden[2] = mKl;

                //Filter
            mPosFilterCoeffU[0] = posnum[2]*(mTimestep*mTimestep) - 2.0*posnum[1]*mTimestep + 4.0*posnum[0];
            mPosFilterCoeffU[1] = 2.0*posnum[2]*(mTimestep*mTimestep) - 8.0*posnum[0];
            mPosFilterCoeffU[2] = posnum[2]*(mTimestep*mTimestep) + 2.0*posnum[1]*mTimestep + 4.0*posnum[0];

            mPosFilterCoeffY[0] = posden[2]*(mTimestep*mTimestep) - 2.0*posden[1]*mTimestep + 4.0*posden[0];
            mPosFilterCoeffY[1] = 2.0*posden[2]*(mTimestep*mTimestep) - 8.0*posden[0];
            mPosFilterCoeffY[2] = posden[2]*(mTimestep*mTimestep) + 2.0*posden[1]*mTimestep + 4.0*posden[0];

            *x2 = 1.0/mPosFilterCoeffY[2]*(mPosFilterCoeffU[2] * (cx1 - *cx2) + mPosFilterCoeffU[1]*mPosFilterDelayU[0] + mPosFilterCoeffU[0]*mPosFilterDelayU[1] - (mPosFilterCoeffY[1]*mPosFilterDelayY[0] + mPosFilterCoeffY[0]*mPosFilterDelayY[1]));

            if (*x2 > mStroke)
            {
                mPosFilterDelayU[1] = mStroke;
                mPosFilterDelayU[0] = mStroke;
                mPosFilterDelayY[1] = mStroke;
                mPosFilterDelayY[0] = mStroke;
                *x2 = mStroke;
            }
            else if (*x2 < 0)
            {
                mPosFilterDelayU[1] = 0;
                mPosFilterDelayU[0] = 0;
                mPosFilterDelayY[1] = 0;
                mPosFilterDelayY[0] = 0;
                *x2 = 0;
            }
            else
            {
                mPosFilterDelayU[1] = mPosFilterDelayU[0];
                mPosFilterDelayU[0] = (cx1 - *cx2);
                mPosFilterDelayY[1] = mPosFilterDelayY[0];
                mPosFilterDelayY[0] = *x2;
            }

                //

            velnum[0] = 0.0;
            velnum[1] = 1.0;
            velnum[2] = 0.0;
            velden[0] = 0.0;
            velden[1] = mTao;
            velden[2] = 1.0;

                //Filter
            mVelFilterCoeffU[0] = velnum[2]*(mTimestep*mTimestep) - 2.0*velnum[1]*mTimestep + 4.0*velnum[0];
            mVelFilterCoeffU[1] = 2.0*velnum[2]*(mTimestep*mTimestep) - 8.0*velnum[0];
            mVelFilterCoeffU[2] = velnum[2]*(mTimestep*mTimestep) + 2.0*velnum[1]*mTimestep + 4.0*velnum[0];

            mVelFilterCoeffY[0] = velden[2]*(mTimestep*mTimestep) - 2.0*velden[1]*mTimestep + 4.0*velden[0];
            mVelFilterCoeffY[1] = 2.0*velden[2]*(mTimestep*mTimestep) - 8.0*velden[0];
            mVelFilterCoeffY[2] = velden[2]*(mTimestep*mTimestep) + 2.0*velden[1]*mTimestep + 4.0*velden[0];

            *v2 = 1.0/mVelFilterCoeffY[2]*(mVelFilterCoeffU[2] * *x2 + mVelFilterCoeffU[1]*mVelFilterDelayU[0] + mVelFilterCoeffU[0]*mVelFilterDelayU[1] - (mVelFilterCoeffY[1]*mVelFilterDelayY[0] + mVelFilterCoeffY[0]*mVelFilterDelayY[1]));

            mVelFilterDelayU[1] = mVelFilterDelayU[0];
            mVelFilterDelayU[0] = *x2;
            mVelFilterDelayY[1] = mVelFilterDelayY[0];
            mVelFilterDelayY[0] = *v2;
                //

//            mVelocityFilter.setNumDen(velnum, velden);
//            *v2 = mVelocityFilter.value(*x2);

            v1 = -*v2;
            *F2 = *cx2 + *Zc2 * *v2;

            //Volumes
            *q1 = mArea1 * v1;
            *q2 = mArea2 * *v2;
            *p1 = *c1 + *Zc1 * *q1;
            *p2 = *c2 + *Zc2 * *q2;
        }
    };


}

#endif // HYDRAULICCYLINDERQ_HPP_INCLUDED
