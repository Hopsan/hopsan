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

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
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
    double mDeadVolume1;
    double mDeadVolume2;
    double mStroke;
    double mEquivalentMass;
    double mBp;
    double mBetae;
    double mZc1;
    double mZc2;
    double mZx;
    double mAlfaSpring;
    Delay mDelayedC1prim, mDelayedC2prim, mDelayedV, mDelayedCxSpring;
    Port *mpP1, *mpP2, *mpP3;

public:
    static Component *Creator()
    {
        return new HydraulicCylinderC("CylinderC");
    }

    HydraulicCylinderC(const string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "HydraulicCylinderC";
        mStartPosition = 0.0;
        mStartVelocity = 0.0;
        mStartPressure1 = 0;
        mStartPressure2 = 0;
        mArea1 = 1.0e-3;
        mArea2 = 1.0e-3;
        mStroke = 1.0;
        mEquivalentMass = 1000.0;
        mDeadVolume1 = 0.01;            //3.0e-4;
        mDeadVolume2 = 0.01;            //3.0e-4;
        mBp = 10;
        mBetae = 1.0e9;
        mAlfaSpring = 0.5;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
        mpP3 = addPowerPort("P3", "NodeMechanic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("x0", "Initial Position", "[m]", mStartPosition);
        registerParameter("v0", "Initial Velocity", "[m]", mStartVelocity);
        registerParameter("Area1", "Piston Area 1", "[m^2]", mArea1);
        registerParameter("Area2", "Piston Area 2", "[m^2]", mArea2);
        registerParameter("Stroke", "Stroke", "[m]", mStroke);
        registerParameter("EquivalentMass", "Equivalent Load Mass", "[kg]", mEquivalentMass);
        registerParameter("DeadVolume1", "Dead Volume in Chamber 1", "[m^3]", mDeadVolume1);
        registerParameter("DeadVolume2", "Dead Volume in Chamber 2", "[m^3]", mDeadVolume2);
        registerParameter("Bp", "Damping Coefficient", "[Ns/m]", mBp);
        registerParameter("Betae", "Bulk Modulus", "[Pa]", mBetae);
    }


    void initialize()
    {
        cout << "Startposition = " << mStartPosition << endl;
        mDelayedC1prim.setStepDelay(1);
        mDelayedC2prim.setStepDelay(1);
        mDelayedV.setStepDelay(1);
        mDelayedCxSpring.setStepDelay(1);

        mDelayedC1prim.initialize(mTime, mStartPressure1);
        mDelayedC2prim.initialize(mTime, mStartPressure2);
        mDelayedV.initialize(mTime, mStartVelocity);
        mDelayedCxSpring.initialize(mTime, 0.0);

        mZx = mZc2*pow(mArea1,2) + mZc2*pow(mArea2,2) + mBp;

        mVolume1 = mDeadVolume1 + mStartPosition*mArea1;
        mVolume2 = mDeadVolume2 + (mStroke-mStartPosition)*mArea2;
        mZc1 = mBetae/mVolume1*mTimestep;
        mZc2 = mBetae/mVolume2*mTimestep;

        //Write to nodes
        mpP1->writeNode(NodeHydraulic::MASSFLOW,     mStartVelocity*mArea1);
        mpP1->writeNode(NodeHydraulic::PRESSURE,     mStartPressure1);
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure1 + mZc1*mStartVelocity*mArea1);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc1);
        mpP2->writeNode(NodeHydraulic::MASSFLOW,     mStartVelocity*mArea2);
        mpP2->writeNode(NodeHydraulic::PRESSURE,     mStartPressure2);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure2 + mZc1*mStartVelocity*mArea2);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc2);
        mpP3->writeNode(NodeMechanic::POSITION,      mStartPosition-mStroke);
        mpP3->writeNode(NodeMechanic::VELOCITY,      -mStartVelocity);
        mpP3->writeNode(NodeMechanic::WAVEVARIABLE, mArea1*mStartPressure1 + mArea2*mStartPressure2);
        mpP3->writeNode(NodeMechanic::CHARIMP,      mZx);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q1  = mpP1->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double q2  = mpP2->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double x   = mpP3->readNode(NodeMechanic::POSITION);
        double v   = mpP3->readNode(NodeMechanic::VELOCITY);
        double xPiston = x + mStroke;
        double vPiston = -v;

        //CylinderC equations
        double a = 0.9;
        double delayedQ1prim = -mArea1*mDelayedV.value();
        double delayedQ2prim = mArea2*mDelayedV.value();

            //Volume Calculations
        mVolume1 = mDeadVolume1 + xPiston*mArea1;
        mVolume2 = mDeadVolume2 + (mStroke-xPiston)*mArea2;
        if ( mVolume1 < mDeadVolume1 )
        {
            mVolume1 = mDeadVolume1;
        }
        if ( mVolume2 < mDeadVolume2 )
        {
            mVolume2 = mDeadVolume2;
        }

            //Characteristic Impedances
        mZc1 = mBetae/mVolume1*mTimestep;
        mZc2 = mBetae/mVolume2*mTimestep;


            //Wave variable for volume 1
        double c10 = mDelayedC1prim.value() + 2*mZc1*delayedQ1prim;
        double c1prim0 = c1 + 2*mZc1*q1;
        c1 = a*c1 + (1-a)*c10;
        double c1prim = a*mDelayedC1prim.value() + (1-a)*c1prim0;

            //Wave variable for volume 2
        double c20 = mDelayedC2prim.value() + 2*mZc2*delayedQ2prim;
        double c2prim0 = c2 + 2*mZc2*q2;
        c2 = a*c2 + (1-a)*c20;
        double c2prim = a*mDelayedC2prim.value() + (1-a)*c2prim0;


            //End of stroke simulation
        //! @todo Verify these equation
        //--------------------------------------------------------------------------------------//
          double cxSpring, cxSpring0, ZxSpring;
        //double Zx0 = 0.01*mEquivalentMass/mTimestep;
        double kSpring = 30.0*mEquivalentMass;
        if ( xPiston > mStroke )
        {
            ZxSpring = kSpring*mTimestep;
            cxSpring0 = -2.0*ZxSpring*vPiston;
        }
        else if ( xPiston < 0.0 )
        {
            ZxSpring = kSpring*mTimestep;
            cxSpring0 = -2.0*ZxSpring*vPiston;
        }
        else
        {
            ZxSpring = 0.0;
            cxSpring0 = 0.0;
        }

            //Old spring equations
//        if (xPiston > mStroke)
//        {
//            ZxSpring = Zx0/(1.0-mAlfaSpring);
//            cxSpring0 = Zx0/mTimestep * (xPiston - mStroke) - ZxSpring * v;
//        }
//        else if (xPiston < 0.0)
//        {
//            ZxSpring = Zx0/(1.0-mAlfaSpring);
//            cxSpring0 = Zx0/mTimestep * xPiston - ZxSpring * v;
//        }
//        else
//        {
//            ZxSpring = 0.0;
//            cxSpring0 = 0.0;
//        }

        cxSpring = mDelayedCxSpring.value() + cxSpring0;
        //cout << "cxSpring0 = " << cxSpring0 << ",  mDelayedCxSpring = " << mDelayedCxSpring.value() << ",  ZxSpring = " << ZxSpring << endl;
        //--------------------------------------------------------------------------------------//


            //Piston
        double cx = mArea1*c1prim - mArea2*c2prim + cxSpring;
        mZx = mZc1*pow(mArea1,2.0) + mZc2*pow(mArea1,2.0) + mBp + ZxSpring;
        //cout << mTime << ",  xPiston = " << xPiston << ",  cxSpring = " << cxSpring << ",  ZxSpring = " << ZxSpring << endl;

            //Update delays
        mDelayedC1prim.update(c1prim);
        mDelayedC2prim.update(c2prim);
        mDelayedV.update(vPiston);
        mDelayedCxSpring.update(cxSpring);

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc1);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc2);
        mpP3->writeNode(NodeMechanic::WAVEVARIABLE, cx);
        mpP3->writeNode(NodeMechanic::CHARIMP,      mZx);
    }
};

#endif // HYDRAULICCYLINDERC_HPP_INCLUDED
