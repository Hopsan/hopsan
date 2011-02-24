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
        double dp, Bm, cim, J;
        //Integrator mFirstIntegrator;
        //Integrator mSecondIntegrator;
        DoubleIntegratorWithDamping mIntegrator;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_t3, *mpND_a3, *mpND_w3, *mpND_c3, *mpND_Zx3;

        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicFixedDisplacementMotorQ("FixedDisplacementMotorQ");
        }

        HydraulicFixedDisplacementMotorQ(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicFixedDisplacementMotorQ";
            dp = 0.00005;
            Bm = 0;
            cim = 0;
            J = 1;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");

            registerParameter("Dp", "Displacement", "m^3/rev", dp);
            registerParameter("Bm", "Viscous Friction", "?", Bm);       //! @todo Figure out these units
            registerParameter("Cim", "Leakage Coefficient", "?", cim);
            registerParameter("J", "Inerteia Load", "?", J);

        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            mpND_t3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::TORQUE);
            mpND_a3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::ANGLE);
            mpND_w3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::CHARIMP);

            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, t3, a3, w3, c3, Zx3;
            double ble, gamma, c1a, c2a, ct, q1a, q2a, q1leak, q2leak;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            c3 = (*mpND_c3);
            Zx3 = (*mpND_Zx3);

            //Motor equations
            dp = dp / (3.1415 * 2);
            //dpe = dpr * eps;       //For variable displacement motor
            ble = Bm + Zc1 * dp*dp + Zc2 * dp*dp + Zx3;
            gamma = 1 / (cim * (Zc1 + Zc2) + 1);
            c1a = (cim * Zc2 + 1) * gamma * c1 + cim * gamma * Zc1 * c2;
            c2a = (cim * Zc1 + 1) * gamma * c2 + cim * gamma * Zc2 * c1;
            ct = c1a * dp - c2a * dp - c3;
            mIntegrator.setDamping(ble / J * mTimestep);
            mIntegrator.integrate(ct/J);
            w3 = mIntegrator.valueFirst();
            a3 = mIntegrator.valueSecond();

            //Ideal Flow
            q1a = -dp * w3;
            q2a = -q1a;
            p1 = c1a + gamma * Zc1 * q1a;
            p2 = c2a + gamma * Zc2 * q2a;

            //Leakage Flow
            q1leak = -cim * (p1 - p2);
            q2leak = -q1leak;

            //Effective Flow
            q1 = q1a + q1leak;
            q2 = q2a + q2leak;

            //Cavitation Check
            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }

            t3 = c3 + w3 * Zx3;

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_t3) = t3;
            (*mpND_a3) = a3;
            (*mpND_w3) = w3;
        }
    };
}

#endif // HYDRAULICFIXEDDISPLACEMENTMOTORQ_H
