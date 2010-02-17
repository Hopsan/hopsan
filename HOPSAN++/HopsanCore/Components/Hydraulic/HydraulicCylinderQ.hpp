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
    enum {P1, P2, P3};

public:
    static Component *Creator()
    {
        std::cout << "running cylinderq creator" << std::endl;
        return new HydraulicCylinderQ("DefaultCylinderQName");
    }

    HydraulicCylinderQ(const string name,
                       const double area1       = 0.0001,
                       const double area2       = 0.0001,
                       const double stroke      = 0.01,
                       const double bp          = 0.0,
                       const double mass        = 0.05,
                       const double bl          = 0,
                       const double kl          = 1000,
                       const double timestep    = 0.001)
        : ComponentQ(name, timestep)
    {
        mTypeName = "HydraulicCylinderQ";
        mArea1  = area1;
        mArea2  = area2;
        mStroke = stroke;
        mMass   = mass;
        mBp     = bp;
        mBl     = bl;
        mKl     = kl;
        mTao    = 3.0/2.0*mTimestep;        //Velocity filter time constant

        addPowerPort("P1", "NodeHydraulic", P1);
        addPowerPort("P2", "NodeHydraulic", P2);
        addPowerPort("P3", "NodeMechanic", P3);

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

        double x2 = mPortPtrs[P3]->readNode(NodeMechanic::POSITION);
        double v2 = mPortPtrs[P3]->readNode(NodeMechanic::VELOCITY);
        //double c1 = mPortPtrs[P1]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeMechanic::CHARIMP);
        //double c2 = mPortPtrs[P2]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc2 = mPortPtrs[P2]->readNode(NodeMechanic::CHARIMP);
        //double cx1 = mArea1*c1 - mArea2*c2;
        double Zx1 = pow(mArea1,2)*Zc1 + pow(mArea2,2)*Zc2-mBp;
        double cx2 = mPortPtrs[P3]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zx2 = mPortPtrs[P3]->readNode(NodeMechanic::CHARIMP);

        double posnum [3] = {0.0, 0.0, 1.0};
        double posden [3] = {mMass, mBl+Zx1+Zx2, mKl};
        double velnum [3] = {0.0, 1.0, 0.0};
        double velden [3] = {0.0, mTao, 1.0};
        mPositionFilter.initialize(mTime, mTimestep, posnum, posden, cx2, x2, 0.0, mStroke);
        mVelocityFilter.initialize(mTime, mTimestep, velnum, velden, x2, v2);

        //mPositionFilter.update(cx1-cx2);
        //mVelocityFilter.update(cx1-cx2);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeHydraulic::CHARIMP);
        double c2 = mPortPtrs[P2]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mPortPtrs[P2]->readNode(NodeHydraulic::CHARIMP);
        double cx2 = mPortPtrs[P3]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zx2 = mPortPtrs[P3]->readNode(NodeHydraulic::CHARIMP);

        //CylinderCtest Equations

            //Internal mechanical port
        double cx1 = mArea1*c1 - mArea2*c2;
        double Zx1 = pow(mArea1,2)*Zc1 + pow(mArea2,2)*Zc2-mBp;

            //Piston
        double posnum [3] = {0.0, 0.0, 1.0};
        double posden [3] = {mMass, mBl+Zx1+Zx2, mKl};
        mPositionFilter.setNumDen(posnum, posden);
        double x2 = mPositionFilter.value(cx1-cx2);

        double velnum [3] = {0.0, 1.0, 0.0};
        double velden [3] = {0.0, mTao, 1.0};
        mVelocityFilter.setNumDen(velnum, velden);
        double v2 = mVelocityFilter.value(x2);

        double v1 = -v2;
        double F2 = cx2 + Zc2*v2;

            //Volumes
        double q1 = mArea1*v1;
        double q2 = mArea2*v2;
        double p1 = c1 + Zc1*q1;
        double p2 = c2 + Zc2*q2;

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q1);
        mPortPtrs[P2]->writeNode(NodeHydraulic::PRESSURE, p2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::MASSFLOW, q2);
        mPortPtrs[P3]->writeNode(NodeMechanic::POSITION, x2);
        mPortPtrs[P3]->writeNode(NodeMechanic::VELOCITY, v2);
        mPortPtrs[P3]->writeNode(NodeMechanic::FORCE, F2);
    }
};

#endif // HYDRAULICCYLINDERQ_HPP_INCLUDED
