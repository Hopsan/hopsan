//!
//! @file   HydraulicCylinderQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Hydraulic Cylinder of Q type with mass load
//!
//$Id$

//Translated from pyHOPSAN, originally created by someone else

#ifndef HYDRAULICCYLINDERQ_HPP_INCLUDED
#define HYDRAULICCYLINDERQ_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"
//#include "math.h"
//#include "CoreUtilities/TurbulentFlowFunction.h"

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
    TransferFunction mPositionFilterLP2;
    TransferFunction mVelocityFilterLP2;
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
        mArea1  = area1;
        mArea2  = area2;
        mStroke = stroke;
        mMass   = mass;
        mBp     = bp;
        mBl     = bl;
        mKl     = kl;

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

        double c1 = mPortPtrs[P1]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeMechanic::CHARIMP);
        double c2 = mPortPtrs[P2]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc2 = mPortPtrs[P2]->readNode(NodeMechanic::CHARIMP);
        double cx1 = mArea1*c1 - mArea2*c2;
        double Zx1 = pow(mArea1,2)*Zc1 + pow(mArea2,2)*Zc2-mBp;
        double cx2 = mPortPtrs[P3]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zx2 = mPortPtrs[P3]->readNode(NodeMechanic::CHARIMP);


        double posnum [3] = {1.0, 0.0, 0.0};
        double velnum [3] = {0.0, 1.0, 0.0};
        double den [3] = {mKl, mBl+Zx1+Zx2, mMass};

        mPositionFilterLP2.initialize(cx1,cx2, mTime);
        mVelocityFilterLP2.initialize(cx1,cx2, mTime);

        mPositionFilterLP2.setCoefficients(posnum, den, mTimestep);
        mVelocityFilterLP2.setCoefficients(posnum, den, mTimestep);

        mPositionFilterLP2.update(cx1-cx2);
        mVelocityFilterLP2.update(cx1-cx2);
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


        //CylinderC Equations

            //Internal mechanical port
        double cx1 = mArea1*c1 - mArea2*c2;
        double Zx1 = pow(mArea1,2)*Zc1 + pow(mArea2,2)*Zc2-mBp;

            //Piston
        double den [3] = {mKl, mBl+Zx1+Zx2, mMass};

        double posnum [3] = {1.0, 0.0, 0.0};
        mPositionFilterLP2.setCoefficients(posnum, den, mTimestep);
        //mPositionFilterLP2.update(cx1-cx2);                         //JÄTTEFULT!!!
        double x2 = mPositionFilterLP2.getValue(cx1-cx2);

        double velnum [3] = {0.0, 1.0, 0.0};
        mVelocityFilterLP2.setCoefficients(velnum, den, mTimestep);
        //mVelocityFilterLP2.update(cx1-cx2);
        double v2 = mVelocityFilterLP2.getValue(cx1-cx2);

        double x1 = -x2;
        double v1 = -v2;
        double F1 = cx1 + Zc1*v1;
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
