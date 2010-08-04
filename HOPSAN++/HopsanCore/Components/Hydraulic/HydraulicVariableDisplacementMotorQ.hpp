//!
//! @file   HydraulicFixedDisplacementPump.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-07-01
//!
//! @brief Contains a variable displacement hydraulic motor component with inertia load
//!
//$Id$

#ifndef HYDRAULICVARIABLEDISPLACEMENTMOTORQ_H
#define HYDRAULICVARIABLEDISPLACEMENTMOTORQ_H


#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicVariableDisplacementMotorQ : public ComponentQ
{
private:
    double mDp, mBm, mCim, mJ, mEps;
    //Integrator mFirstIntegrator;
    //Integrator mSecondIntegrator;
    DoubleIntegratorWithDamping mIntegrator;
    Port *mpP1, *mpP2, *mpP3, *mpIn;

public:
    static Component *Creator()
    {
        return new HydraulicVariableDisplacementMotorQ("VariableDisplacementMotorQ");
    }

    HydraulicVariableDisplacementMotorQ(const string name) : ComponentQ(name)
    {
        mTypeName = "HydraulicVariableDisplacementMotorQ";
        mDp = 0.00005;
        mBm = 0;
        mCim = 0;
        mJ = 1;
        mEps = 1;

        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
        mpP3 = addPowerPort("P3", "NodeMechanicRotational");
        mpIn = addReadPort("in", "NodeSignal");

        registerParameter("Dp", "Displacement", "m^3/rev", mDp);
        registerParameter("Bm", "Viscous Friction", "Ns/m", mBm);       //! @todo Figure out these units
        registerParameter("Cim", "Leakage Coefficient", "-", mCim);
        registerParameter("J", "Inerteia Load", "kgm^2", mJ);
        registerParameter("eps", "Displacement Position", "-", mEps);
    }


    void initialize()
    {
        mIntegrator.initialize(mTime, mTimestep, 0, 0, 0, 0);

    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);
        double c3 = mpP3->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc3 = mpP3->readNode(NodeMechanic::CHARIMP);

        //Motor equations

        if(mpIn->isConnected())     //Get eps from signal node if connected
        {
            mEps = mpIn->readNode(NodeSignal::VALUE);
        }
        if (mEps > 1)       //Limit of displacement position
        {
            mEps = 1;
        }
        else if (mEps < -1)
        {
            mEps = -1;
        }

        double dp = mDp / (3.1415 * 2) * mEps;
        double ble = mBm + Zc1 * pow(dp,2) + Zc2 * pow(dp,2) + Zc3;
        double gamma = 1 / (mCim * (Zc1 + Zc2) + 1);
        double c1a = (mCim * Zc2 + 1) * gamma * c1 + mCim * gamma * Zc1 * c2;
        double c2a = (mCim * Zc1 + 1) * gamma * c2 + mCim * gamma * Zc2 * c1;
        double ct = c1a * dp - c2a * dp - c3;
        mIntegrator.setDamping(ble / mJ * mTimestep);
        double omega3 = mIntegrator.valueFirst(ct/mJ);
        double phi3 = mIntegrator.valueSecond(ct/mJ);
        mIntegrator.update(ct/mJ);

            //Ideal Flow
        double q1a = -dp * omega3;
        double q2a = -q1a;
        double p1 = c1a + gamma * Zc1 * q1a;
        double p2 = c2a + gamma * Zc2 * q2a;

            //Leakage Flow
        double q1leak = -mCim * (p1 - p2);
        double q2leak = -q1leak;

            //Effective Flow
        double q1 = q1a + q1leak;
        double q2 = q2a + q2leak;

            //Cavitation Check
        if (p1 < 0.0) { p1 = 0.0; }
        if (p2 < 0.0) { p2 = 0.0; }

        double t3 = c3 + omega3 * Zc3;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);
        mpP3->writeNode(NodeMechanicRotational::TORQUE, t3);
        mpP3->writeNode(NodeMechanicRotational::ANGLE, phi3);
        mpP3->writeNode(NodeMechanicRotational::ANGULARVELOCITY, omega3);
    }
};

#endif // HYDRAULICVARIABLEDISPLACEMENTMOTORQ_H
