//!
//! @file   HydraulicCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-20
//!
//! @brief Contains a Hydraulic Cylinder of C type
//!
//$Id$

//Translated from pyHOPSAN, originally created by someone else

#ifndef HYDRAULICCYLINDERC_HPP_INCLUDED
#define HYDRAULICCYLINDERC_HPP_INCLUDED

#include "HopsanCore.h"

class HydraulicCylinderC : public ComponentC
{

private:
    double mStartPressure1;
    double mStartPressure2;
    double mStartPosition;
    double mStartVelocity;
    double mStartAcceleration;
    double mStartForce;
    double mArea1;
    double mArea2;
    double mVolume1;
    double mVolume2;
    double mBp;
    double mBetae;
    double mZc1;
    double mZc2;
    double mZx;
    Delay mDelayedC1prim, mDelayedC2prim, mDelayedV;
    enum {P1, P2, P3};

public:
    static Component *Creator()
    {
        std::cout << "running cylinderc creator" << std::endl;
        return new HydraulicCylinderC("DefaultCylinderCName");
    }

    HydraulicCylinderC(const string name,
                       const double startposition   = 0.0,
                       const double startvelocity   = 0.0,
                       const double area1           = 1.0e-3,
                       const double area2           = 1.0e-3,
                       const double volume1         = 1.0e-3,
                       const double volume2         = 1.0e-3,
                       const double bp              = 10,
                       const double betae           = 1.0e9,
                       const double timestep        = 0.001)
    : ComponentC(name, timestep)
    {
        //Set member attributes
        mStartPosition = startposition;
        mStartVelocity = startvelocity;
        mStartPressure1 = 0;
        mStartPressure2 = 0;
        mArea1 = area1;
        mArea2 = area2;
        mVolume1 = volume1;
        mVolume2 = volume2;
        mBp = bp;
        mBetae = betae;
        mZc1 = mBetae/mVolume1*mTimestep;
        mZc2 = mBetae/mVolume2*mTimestep;

        //Add ports to the component
        addPowerPort("P1", "NodeHydraulic", P1);
        addPowerPort("P2", "NodeHydraulic", P2);
        addPowerPort("P3", "NodeMechanic", P3);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Area1", "Piston Area 1", "[m^2]", mArea1);
        registerParameter("Area2", "Piston Area 2", "[m^2]", mArea2);
        registerParameter("Volume1", "Cylinder Volume 1", "[m^3]", mVolume1);
        registerParameter("Volume2", "Cylinder Volume 2", "[m^3]", mVolume2);
        registerParameter("Bp", "Damping Coefficient", "[Ns/m]", mBp);
        registerParameter("Betae", "Bulk Modulus", "[Pa]", mBetae);
    }

	void initialize()
    {

        mDelayedC1prim.initialize(mTime, mStartPressure1);
        mDelayedC2prim.initialize(mTime, mStartPressure2);
        mDelayedV.initialize(mTime, mStartVelocity);

        mZx = mZc2*pow(mArea1,2) + mZc2*pow(mArea2,2) + mBp;

        //Write to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW,     mStartVelocity*mArea1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE,     mStartPressure1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure1 + mZc1*mStartVelocity*mArea1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::CHARIMP,      mZc1);
        mPortPtrs[P2]->writeNode(NodeHydraulic::MASSFLOW,     mStartVelocity*mArea2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::PRESSURE,     mStartPressure2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure2 + mZc1*mStartVelocity*mArea2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::CHARIMP,      mZc2);
        mPortPtrs[P3]->writeNode(NodeMechanic::POSITION,      mStartPosition);
        mPortPtrs[P3]->writeNode(NodeMechanic::VELOCITY,      mStartVelocity);
        mPortPtrs[P3]->writeNode(NodeHydraulic::WAVEVARIABLE, mArea1*mStartPressure1 + mArea2*mStartPressure2);
        mPortPtrs[P3]->writeNode(NodeHydraulic::CHARIMP,      mZx);
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q1  = mPortPtrs[P1]->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double q2  = mPortPtrs[P2]->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mPortPtrs[P2]->readNode(NodeHydraulic::WAVEVARIABLE);
        double v   = mPortPtrs[P3]->readNode(NodeMechanic::VELOCITY);

        //CylinderC equations
        double a = 0.9;
        double delayedQ1prim = mArea1*mDelayedV.value();
        double delayedQ2prim = -mArea2*mDelayedV.value();

            //Volume 1
        double c10 = mDelayedC1prim.value() + 2*mZc1*delayedQ1prim;
        double c1prim0 = c1 + 2*mZc1*q1;
        c1 = a*c1 + (1-a)*c10;
        double c1prim = a*mDelayedC1prim.value() + (1-a)*c1prim0;

            //Volume 2
        double c20 = mDelayedC2prim.value() + 2*mZc2*delayedQ2prim;
        double c2prim0 = c2 + 2*mZc2*q2;
        c2 = a*c2 + (1-a)*c20;
        double c2prim = a*mDelayedC2prim.value() + (1-a)*c2prim0;

            //Piston
        double cx = mArea1*c1prim - mArea2*c2prim;
        mZx = mZc1*pow(mArea1,2) + mZc2*pow(mArea1,2) + mBp;

            //Update delays
        mDelayedC1prim.update(c1prim);
        mDelayedC2prim.update(c2prim);
        mDelayedV.update(v);

     //   if (mTime < 0.1)
     //   {
     //       cout << "p1 = " << p1 << ", p2 = " << p2 << ", q1 = " << q1 << endl;
     //   }

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::CHARIMP,      mZc1);
        mPortPtrs[P2]->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::CHARIMP,      mZc2);
        mPortPtrs[P3]->writeNode(NodeHydraulic::WAVEVARIABLE, cx);
        mPortPtrs[P3]->writeNode(NodeHydraulic::CHARIMP,      mZx);
    }
};

#endif // HYDRAULICCYLINDERC_HPP_INCLUDED
