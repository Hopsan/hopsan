//!
//! @file   HydraulicFixedDisplacementPump.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contains a hydraulic motor component with inertia load
//!
//$Id$

#ifndef HYDRAULICFIXEDDISPLACEMENTMOTORQ_H
#define HYDRAULICFIXEDDISPLACEMENTMOTORQ_H


#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicFixedDisplacementMotorQ : public ComponentQ
    {
    private:
        double mDp, mBm, mCim, mJ;
        //Integrator mFirstIntegrator;
        //Integrator mSecondIntegrator;
        DoubleIntegratorWithDamping mIntegrator;
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicFixedDisplacementMotorQ("FixedDisplacementMotorQ");
        }

        HydraulicFixedDisplacementMotorQ(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicFixedDisplacementMotorQ";
            mDp = 0.00005;
            mBm = 0;
            mCim = 0;
            mJ = 1;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");

            registerParameter("Dp", "Displacement", "m^3/rev", mDp);
            registerParameter("Bm", "Viscous Friction", "?", mBm);       //! @todo Figure out these units
            registerParameter("Cim", "Leakage Coefficient", "?", mCim);
            registerParameter("J", "Inerteia Load", "?", mJ);

        }


        void initialize()
        {
            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);
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

            double dp = mDp / (3.1415 * 2);
            //double dpe = dpr * eps;       //For variable displacement motor
            double ble = mBm + Zc1 * dp*dp + Zc2 * dp*dp + Zc3;
            double gamma = 1 / (mCim * (Zc1 + Zc2) + 1);
            double c1a = (mCim * Zc2 + 1) * gamma * c1 + mCim * gamma * Zc1 * c2;
            double c2a = (mCim * Zc1 + 1) * gamma * c2 + mCim * gamma * Zc2 * c1;
            double ct = c1a * dp - c2a * dp - c3;
            mIntegrator.setDamping(ble / mJ * mTimestep);
            mIntegrator.integrate(ct/mJ);
            double omega3 = mIntegrator.valueFirst();
            double phi3 = mIntegrator.valueSecond();


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
            mpP1->writeNode(NodeHydraulic::FLOW, q1);
            mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
            mpP2->writeNode(NodeHydraulic::FLOW, q2);
            mpP3->writeNode(NodeMechanicRotational::TORQUE, t3);
            mpP3->writeNode(NodeMechanicRotational::ANGLE, phi3);
            mpP3->writeNode(NodeMechanicRotational::ANGULARVELOCITY, omega3);
        }
    };
}

#endif // HYDRAULICFIXEDDISPLACEMENTMOTORQ_H
