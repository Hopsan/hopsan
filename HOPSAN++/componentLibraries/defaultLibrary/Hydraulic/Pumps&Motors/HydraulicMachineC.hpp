/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicMachineC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-04
//!
//! @brief Contains a Hydraulic Machine of C type
//!
//$Id$

//Translated from old Hopsan, originally created by someone else

#ifndef HYDRAULICMACHINEC_HPP_INCLUDED
#define HYDRAULICMACHINEC_HPP_INCLUDED



#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicMachineC : public ComponentC
    {

    private:
        double cp1, cp2, v1min, v2min, alfa, wfak, je;
        double *mpBetae, *mpV1, *mpV2, *mpDm, *mpClm, *mpBm;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_t3, *mpND_a3, *mpND_w3,*mpND_c3, *mpND_Zx3, *mpEps;

        Delay mDelayedC1, mDelayedC2, mDelayedCp1, mDelayedCp2, mDelayedCp1e, mDelayedCp2e;
        Port *mpP1, *mpP2, *mpP3, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicMachineC();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");

            addInputVariable("eps", "Displacement setting", "", 1, &mpEps);
            addInputVariable("Beta_e", "Bulk modulus of oil", "[Pa]", 1000000000, &mpBetae);
            addInputVariable("V_1", "Volume at port 1", "[m^3]", 0.005, &mpV1);
            addInputVariable("V_2", "Volume at port 2", "[m^3]", 0.005, &mpV2);
            addInputVariable("D_m", "Displacement", "[m^3/rev]", 0.00005, &mpDm);
            addInputVariable("C_lm", "Leakage coefficient", "[]", 0.0, &mpClm);
            addInputVariable("B_m", "Viscous friction coefficient", "[Nms/rad]", 0.0, &mpBm);

            addConstant("J_em", "Equivalent load of inertia", "[kg*m^2]", 1, je);

            setStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::Pressure, 1.0e5);
        }


        void initialize()
        {
            //Assign node pointers
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            mpND_t3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Torque);
            mpND_a3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Angle);
            mpND_w3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::AngularVelocity);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::WaveVariable);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::CharImpedance);

            double dpr, dpe, ka, v1e, v2e, qp1, qp2;

            //Read input variables from nodes
            double p1 = (*mpND_p1);
            double q1 = (*mpND_q1);
            double p2 = (*mpND_p2);
            double q2 = (*mpND_q2);
            double t3 = (*mpND_t3);
            double w3 = (*mpND_w3);
            double eps = (*mpEps);
            double betae = (*mpBetae);
            double v1 = (*mpV1);
            double v2 = (*mpV2);
            double dm = (*mpDm);
            double clm = (*mpClm);
            double bm = (*mpBm);

            //Initialization
            alfa = 0.1;
            wfak = 0.1;
            dpr = dm / (pi * 2);
            dpe = dpr * eps;
            ka = 1 / (1 - alfa);
            v1min = 2*ka*betae * mTimestep*mTimestep * dpr*dpr / (2*wfak*je);
            v2min = 2*ka*betae * mTimestep*mTimestep * dpr*dpr / (2*wfak*je);
            v1e = v1;
            v2e = v2;
            if (v1e < v1min) { v1e = v1min; }
            if (v2e < v2min) { v2e = v2min; }
            double Zc1 = 2 * ka*betae*mTimestep / (2*v1e);
            double Zc2 = 2 * ka*betae*mTimestep / (2*v2e);
            mDelayedC1.initialize(1, p1-Zc1*q1);
            double c1 = p1 - Zc1*q1;
            mDelayedC2.initialize(1, p2-Zc2*q2);
            double c2 = p2 - Zc2*q2;
            qp1 = dpe * w3;
            qp2 = -dpe * w3;

            cp1 = p1 - Zc1 * (qp1 - clm * (p1 - p2));
            cp2 = p2 - Zc2 * (qp2 - clm * (p2 - p1));
            mDelayedCp1.initialize(1, cp1);
            mDelayedCp2.initialize(1, cp2);
            mDelayedCp1e.initialize(1, cp1);
            mDelayedCp2e.initialize(1, cp2);
            double c3 = t3;
            double Zx3 = dpe*dpe * Zc1 + dpe*dpe * Zc2 + bm;

            //Write output variables from nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
            (*mpND_t3) = t3;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare Local variables
            double cp10e, cp20e, ka, /*ap, */c1e, c2e, p1e, p2e, ct1, ct2,
                   v1e, v2e, pm1, pp1, qp1, qp2, pp2, pm2, cp10, cp20, dpe, dpr, /*wp, */ct1e,
                   ct2e, pm1e, pm2e, pp1e, qp1e, qp2e, pp2e, cp1e, cp2e;

            //Read input variables from nodes
            double p1 = (*mpND_p1);
            double q1 = (*mpND_q1);
            double c1 = (*mpND_c1);
            double p2 = (*mpND_p2);
            double q2 = (*mpND_q2);
            double c2 = (*mpND_c2);
            double w3 = (*mpND_w3);
            double eps = (*mpEps);
            double betae = (*mpBetae);
            double v1 = (*mpV1);
            double v2 = (*mpV2);
            double dm = (*mpDm);
            double clm = (*mpClm);
            double bm = (*mpBm);

            //Machine equations
            v1e = std::max(v1,v1min);
            v2e = std::max(v2,v2min);

            dpr = dm / (pi * 2);
            limitValue(eps, -1.0, +1.0);
            dpe = dpr * eps;    //Effective displacement
            ka = 1 / (1 - alfa);
            //ap = -a3;
            //wp = -w3;
            double Zc1 = 2*ka*betae*mTimestep / (2*v1e);
            double Zc2 = 2*ka*betae*mTimestep / (2*v2e);

            qp1 = dpe*(w3);
            qp2 = -dpe*w3;

            pp1 = (cp1 + qp1*Zc1 + clm*(cp2*Zc1 + cp1*Zc2)) / (clm*(Zc1 + Zc2) + 1);
            pp2 = (cp2 + qp2*Zc2 + clm*(cp2*Zc1 + cp1*Zc2)) / (clm*(Zc1 + Zc2) + 1);
            pp1e = pp1;
            pp2e = pp2;
            if (pp1e < 0.0) { pp1e = 0.0; }
            if (pp2e < 0.0) { pp2e = 0.0; }
            qp1e = qp1 - clm * (pp1e - pp2e);
            qp2e = qp2 - clm * (pp2e - pp1e);

            // Characteristics
            ct1 = 0.0;
            ct1 = ct1 + c1 + 2*Zc1*q1;
            ct1 = ct1 + pp1 + Zc1*qp1e;
            pm1 = ct1/2.0;

            ct2 = 0.0;
            ct2 = ct2 + c2 + 2*Zc2*q2;
            ct2 = ct2 + pp2 + Zc2*qp2e;
            pm2 = ct2/2.0;

            cp10 = 2*pm1 - pp1 - Zc1*qp1e;
            cp1 = alfa*mDelayedCp1.update(cp1) + (1 - alfa)*cp10;
            c1e = 2*pm1 - c1 - 2*Zc1*q1;
            c1 = alfa*mDelayedC1.update(c1) + (1 - alfa)*c1e;

            cp20 = 2*pm2 - pp2 - Zc2*qp2e;
            cp2 = alfa*mDelayedCp2.update(cp2) + (1 - alfa)*cp20;
            c2e = 2*pm2 - c2 - 2*Zc2*q2;
            c2 = alfa*mDelayedC2.update(c2) + (1 - alfa)*c2e;

            // Effective characteristics
            p1e = p1;
            if (p1e < 0.0) { p1e = 0.0; }
            ct1e = 0.0;
            ct1e = ct1e + p1e + Zc1*q1;
            ct1e = ct1e + pp1e + Zc1*qp1e;
            pm1e = ct1e/2.0;

            p2e = p2;
            if (p2e < 0.0) { p2e = 0.0; }
            ct2e = 0.0;
            ct2e = ct2e + p2e + Zc2*q2;
            ct2e = ct2e + pp2e + Zc2*qp2e;
            pm2e = ct2e/2.0;

            // Effective characteristics at the motor taking account for cavitation
            cp10e = 2*pm1e - pp1e - Zc1*qp1e;
            cp20e = 2*pm2e - pp2e - Zc2*qp2e;
            cp1e = alfa*mDelayedCp1e.getOldest() + (1 - alfa)*cp10e;
            cp2e = alfa*mDelayedCp2e.getOldest() + (1 - alfa)*cp20e;
            mDelayedCp1e.update(cp1e);
            mDelayedCp2e.update(cp2e);

                // Force characteristics
            double c3 = (cp1e - cp2e)*dpe;
            double Zx3 = dpe*dpe * (Zc1 + Zc2) + bm;

            //Write output variables from nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }
    };
}

#endif // HYDRAULICMACHINEC_HPP_INCLUDED
