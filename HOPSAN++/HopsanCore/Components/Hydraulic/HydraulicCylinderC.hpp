//!
//! @file   HydraulicCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-20
//!
//! @brief Contains a Hydraulic Cylinder of C type
//!
//$Id$

//Translated from old Hopsan, originally created by someone else

#ifndef HYDRAULICCYLINDERC_HPP_INCLUDED
#define HYDRAULICCYLINDERC_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicCylinderC : public ComponentC
    {

    private:
            //Initial values
        double mStartPressure1;
        double mStartPressure2;
        double mStartPosition;
        double mStartVelocity;
        double mStartAcceleration;
        double mStartForce;

            //Changeable parameters
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
        double mZx;
        double mAlphaSpring;
        double mLeakageCoefficient;

            //Local variables
        double mAlphaZc1, mAlphaZc2;
        double mC1Effective, mC2Effective;
        double mC1Internal, mC2Internal;
        double mC1InternalEffective, mC2InternalEffective;
        double mC1Internal0, mC2Internal0;
        double mC1Internal0Effective, mC2Internal0Effective;
        double mClp, mCt1, mCt2, mCt1Effective, mCt2Effective;
        double mP1Effective, mP2Effective;
        double mPm1, mPm2, mPm1Effective, mPm2Effective;
        double mP1Internal, mP2Internal;
        double mP1InternalEffective, mP2InternalEffective;
        double mQ1Internal, mQ2Internal;
        double mQ1InternalEffective, mQ2InternalEffective;
        double mMinVolume1, mMinVolume2;
        double mZc10, mZc20;
        double mXInternal;
        double mVInternal;
        double mWfak;
        double mAlpha;
        double mZSpring, mZSpring0, mCSpring, mCSpring0, mKSpring;

        double mPrevC1, mPrevC2;
        double mPrevZc1, mPrevZc2;
        double mPrevC1Internal, mPrevC2Internal;
        double mPrevC1InternalEffective, mPrevC2InternalEffective;
        double mPrevCSpring;

        Port *mpP1, *mpP2, *mpP3;//, *mpDebug1, *mpDebug2;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderC("CylinderC");
        }

        HydraulicCylinderC(const std::string name) : ComponentC(name)
        {
            mAlphaZc1 = 0;
            mAlphaZc2 = 0;
            mC1Effective = 0;
            mC2Effective =0;
            mC1Internal = 0;
            mC2Internal = 0;
            mC1InternalEffective = 0;
            mC2InternalEffective = 0;
            mC1Internal0 = 0;
            mC2Internal0 = 0;
            mC1Internal0Effective = 0;
            mC2Internal0Effective = 0;
            mClp = 0;
            mCt1 = 0;
            mCt2 = 0;
            mCt1Effective = 0;
            mCt2Effective = 0;
            mP1Effective = 0;
            mP2Effective = 0;
            mPm1 = 0;
            mPm2 = 0;
            mPm1Effective = 0;
            mPm2Effective = 0;
            mP1Internal = 0;
            mP2Internal = 0;
            mP1InternalEffective = 0;
            mP2InternalEffective = 0;
            mQ1Internal = 0;
            mQ2Internal = 0;
            mQ1InternalEffective = 0;
            mQ2InternalEffective = 0;
            mMinVolume1 = 0;
            mMinVolume2 = 0;
            mZc10 = 0;
            mZc20 = 0;
            mXInternal = 0;;
            mVInternal = 0;;
            mWfak = 0;;
            mAlpha = 0;;
            mZSpring = 0;
            mZSpring0 = 0;
            mCSpring = 0;
            mCSpring0 = 0;
            mKSpring = 0;

            //Set member attributes
            mTypeName = "HydraulicCylinderC";
            mStartPosition = 0.0;
            mStartVelocity = 0.0;
            mStartForce = 0.0;
            mStartPressure1 = 100000;       //! @todo Don't know if we should assume atmospheric pressure like this
            mStartPressure2 = 100000;
            mArea1 = 1.0e-3;
            mArea2 = 1.0e-3;
            mStroke = 1.0;
            mEquivalentMass = 1000.0;
            mDeadVolume1 = 3.0e-4;
            mDeadVolume2 = 3.0e-4;
            mBp = 10;
            mBetae = 1.0e9;
            mAlphaSpring = 0.5;
            mWfak = 0.1;
            mAlpha = 0.01;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");
            //mpDebug1 = addWritePort("Debug1", "NodeSignal");
            //mpDebug2 = addWritePort("Debug2", "NodeSignal");

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
            registerParameter("Cip", "Leakage Coefficient", "?", mLeakageCoefficient);
        }


        void initialize()
        {
            mZSpring0 = mWfak * mEquivalentMass / mTimestep;

            mMinVolume1 = mBetae * mTimestep*mTimestep  * mArea1*mArea1 / (mWfak * mEquivalentMass);
            mMinVolume2 = mBetae * mTimestep*mTimestep  * mArea2*mArea2 / (mWfak * mEquivalentMass);

            mXInternal = mStartPosition;
            mVInternal = mStartVelocity;

            mVolume1 = mDeadVolume1 + mXInternal*mArea1;
            mVolume2 = mDeadVolume2 + (mStroke-mXInternal)*mArea2;
            if (mVolume1 < mMinVolume1) { mVolume1 = mMinVolume1; }
            if (mVolume2 < mMinVolume2) { mVolume2 = mMinVolume2; }

            mZc10 = mBetae*mTimestep/mVolume1;
            mZc20 = mBetae*mTimestep/mVolume2;

            mPrevZc1 = mZc10;
            mPrevZc2 = mZc20;

            mQ1Internal = -mArea1 * mVInternal;
            mQ2Internal = mArea2 * mVInternal;

            double c1 = mStartPressure1 - mZc10 * mQ1Internal;
            double c2 = mStartPressure2 - mZc20 * mQ2Internal;

            mPrevC1 = c1;
            mPrevC2 = c2;

            mC1Internal = mStartPressure1 - mZc10 * (mQ1Internal - mLeakageCoefficient * (mStartPressure1 - mStartPressure2));
            mC2Internal = mStartPressure2 - mZc20 * (mQ2Internal - mLeakageCoefficient * (mStartPressure2 - mStartPressure1));

            mPrevC1Internal = mC1Internal;
            mPrevC2Internal = mC2Internal;
            mPrevC1InternalEffective = mC1Internal;
            mPrevC2InternalEffective = mC2Internal;

            double c3 = mC1Internal*mArea1 - mC2Internal*mArea2;
            double Zc3 = mZc10*mArea1*mArea1 + mZc20*mArea2*mArea2 + mBp;

            mPrevCSpring = 0.0;

            //Write to nodes
            mpP1->writeNode(NodeHydraulic::FLOW,     mQ1Internal);
            mpP1->writeNode(NodeHydraulic::PRESSURE,     mStartPressure1);
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
            mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc10);
            mpP2->writeNode(NodeHydraulic::FLOW,     mQ2Internal);
            mpP2->writeNode(NodeHydraulic::PRESSURE,     mStartPressure2);
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
            mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc20);
            mpP3->writeNode(NodeMechanic::POSITION,      -mXInternal);
            mpP3->writeNode(NodeMechanic::VELOCITY,      -mVInternal);
            mpP3->writeNode(NodeMechanic::FORCE,        c3);
            mpP3->writeNode(NodeMechanic::WAVEVARIABLE, c3);
            mpP3->writeNode(NodeMechanic::CHARIMP,      Zc3);
        }

        //DEBUG: Initialization is verified against old Hopsan

        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double p1  = mpP1->readNode(NodeHydraulic::PRESSURE);
            double q1  = mpP1->readNode(NodeHydraulic::FLOW);
            double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);

            double p2  = mpP2->readNode(NodeHydraulic::PRESSURE);
            double q2  = mpP2->readNode(NodeHydraulic::FLOW);
            double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

            double x   = mpP3->readNode(NodeMechanic::POSITION);
            double v   = mpP3->readNode(NodeMechanic::VELOCITY);
            double c3  = mpP3->readNode(NodeMechanic::WAVEVARIABLE);
            double Zc3 = mpP3->readNode(NodeMechanic::CHARIMP);

            mXInternal = -x;
            mVInternal = -v;

                // Volumetric impedances
            mVolume1 = mDeadVolume1 + mArea1 * mXInternal;
            mVolume2 = mDeadVolume2 + mArea2 * (mStroke - mXInternal);
            if (mVolume1 < mMinVolume1) { mVolume1 = mMinVolume1; }
            if (mVolume2 < mMinVolume2) { mVolume2 = mMinVolume2; }
            //Checked
            mZc10 = mBetae * mTimestep / mVolume1;
            mZc20 = mBetae * mTimestep / mVolume2;
            mAlphaZc1 = mZc10 / mPrevZc1;
            mAlphaZc2 = mZc20 / mPrevZc2;
            //Checked
            mQ1Internal = -mArea1 * mVInternal;
            mQ2Internal = mArea2 * mVInternal;
            //Checked
            mP1Internal = (mC1Internal + mQ1Internal * mZc10 + mLeakageCoefficient * (mC2Internal*mZc10 + mC1Internal*mZc20 + mQ1Internal*mZc10*mZc20 + mQ2Internal*mZc10*mZc20)) / (mLeakageCoefficient*(mZc10 + mZc20) + 1);
            mP2Internal = (mC2Internal + mQ2Internal * mZc20 + mLeakageCoefficient * (mC2Internal*mZc10 + mC1Internal*mZc20 + mQ1Internal*mZc10*mZc20 + mQ2Internal*mZc10*mZc20)) / (mLeakageCoefficient*(mZc10 + mZc20) + 1);
            //Checked
            mP1InternalEffective = mP1Internal;
            mP2InternalEffective = mP2Internal;
            if (mP1InternalEffective < 0.0) { mP1InternalEffective = 0.0; }
            if (mP2InternalEffective < 0.0) { mP2InternalEffective = 0.0; }
            //Checked
            mQ1InternalEffective = mQ1Internal - mLeakageCoefficient*(mP1InternalEffective - mP2InternalEffective);
            mQ2InternalEffective = mQ2Internal - mLeakageCoefficient*(mP2InternalEffective - mP1InternalEffective);
            //Checked

                // Characteristics
            mCt1 = c1 + mZc10 * 2 * q1;
            mCt1 = mCt1 + mP1Internal + mZc10 * mQ1InternalEffective;
            mPm1 = mCt1 / 2;

            mCt2 = c2 + mZc20 * 2 * q2;
            mCt2 = mCt2 + mP2Internal + mZc20 * mQ2InternalEffective;
            mPm2 = mCt2 / 2;
            //Checked
            mC1Internal0 = mPm1 * (mAlphaZc1 + 1) - mAlphaZc1 * mP1Internal - mAlphaZc1 * mZc10 * mQ1InternalEffective;
            mC1Internal = (1 - mAlpha) * mC1Internal0 + mAlpha * (mPrevC1Internal + (mPrevZc1 - mZc10) * mQ1InternalEffective);
            mPrevC1Internal = mC1Internal;
            //Checked
            mC1Effective = mPm1 * (mAlphaZc1 + 1) - mAlphaZc1 * c1 - mAlphaZc1 * 2 * mPrevZc1 * q1;
            c1 = (1 - mAlpha) * mC1Effective + mAlpha * (mPrevC1 + (mPrevZc1 - mZc10) * q1);
            mPrevC1 = c1;
            Zc1 = mZc10;
            //Checked
            mC2Internal0 = mPm2 * (mAlphaZc2 + 1) - mAlphaZc2 * mP2Internal - mAlphaZc2 * mZc20 * mQ2InternalEffective;
            mC2Internal = (1 - mAlpha) * mC2Internal0 + mAlpha * (mPrevC2Internal + (mPrevZc2 - mZc20) * mQ2InternalEffective);
            mPrevC2Internal = mC2Internal;
            //Checked
            mC2Effective = mPm2 * (mAlphaZc2 + 1) - mAlphaZc2 * c2 - mAlphaZc2 * 2 * mPrevZc2 * q2;
            c2 = (1 - mAlpha) * mC2Effective + mAlpha * (mPrevC2 + (mPrevZc2 - mZc20) * q2);
            mPrevC2 = c2;
            Zc2 = mZc20;
            //Checked

                // Effective characteristics
            mP1Effective = p1;
            if (mP1Effective < 0.0) { mP1Effective = 0.0; }
            mCt1Effective = 0.0;
            mCt1Effective = mCt1Effective + mP1Effective + mZc10 * q1;
            mCt1Effective = mCt1Effective + mP1InternalEffective + mZc10 * mQ1InternalEffective;
            mPm1Effective = mCt1Effective / 2;

            mP2Effective = p2;
            if (mP2Effective < 0.0) { mP2Effective = 0.0; }
            mCt2Effective = 0.0;
            mCt2Effective = mCt2Effective + mP2Effective + mZc20 * q2;
            mCt2Effective = mCt2Effective + mP2InternalEffective + mZc20 * mQ2InternalEffective;
            mPm2Effective = mCt2Effective / 2;
            //Checked

                // Effective characteristics at the piston taking account for cavitation
            mC1Internal0Effective = mPm1Effective * (mAlphaZc1 + 1) - mAlphaZc1 * mP1InternalEffective - mAlphaZc1 * mZc10 * mQ1InternalEffective;
            mC2Internal0Effective = mPm2Effective * (mAlphaZc2 + 1) - mAlphaZc2 * mP2InternalEffective - mAlphaZc2 * mZc20 * mQ2InternalEffective;
            mC1InternalEffective = (1 - mAlpha) * mC1Internal0Effective + mAlpha * (mPrevC1InternalEffective + (mPrevZc1 - mZc10) * mQ1InternalEffective);
            mC2InternalEffective = (1 - mAlpha) * mC2Internal0Effective + mAlpha * (mPrevC2InternalEffective + (mPrevZc2 - mZc20) * mQ2InternalEffective);
            mPrevC1InternalEffective = mC1InternalEffective;
            mPrevC2InternalEffective = mC2InternalEffective;
            mPrevZc1 = mZc10;
            mPrevZc2 = mZc20;
            //Checked
                // Force characteristics

            mAlphaSpring = 0.5;
            if (mXInternal > mStroke)
            {
                mZSpring = mZSpring0 / (1.0 - mAlphaSpring);
                mCSpring0 = -mZSpring0 / mTimestep * (mXInternal - mStroke) - mZSpring0 * mVInternal;
            }
            else if (mXInternal < 0.0)
            {
                mZSpring = mZSpring0 / (1.0 - mAlphaSpring);
                mCSpring0 = -mZSpring0 / mTimestep * mXInternal - mZSpring0 * mVInternal;
            }
            else
            {
                mZSpring = 0.0;
                mCSpring0 = 0.0;
            }

                /* Filtering of the characteristics */

            mCSpring = mAlphaSpring * mPrevCSpring + (1.0 - mAlphaSpring) * mCSpring0;
            mPrevCSpring = mCSpring;

            c3 = mC1InternalEffective*mArea1 - mC2InternalEffective*mArea2 + mCSpring;
            Zc3 = mArea1*mArea1 * mZc10 + mArea2*mArea2 * mZc20 + mBp + mZSpring;

            //mpDebug1->writeNode(NodeSignal::VALUE, mC1Internal);
            //mpDebug2->writeNode(NodeSignal::VALUE, mC2Internal);

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
            mpP1->writeNode(NodeHydraulic::CHARIMP,      Zc1);
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
            mpP2->writeNode(NodeHydraulic::CHARIMP,      Zc2);
            mpP3->writeNode(NodeMechanic::WAVEVARIABLE, c3);
            mpP3->writeNode(NodeMechanic::CHARIMP,      Zc3);
        }
    };
}

#endif // HYDRAULICCYLINDERC_HPP_INCLUDED
