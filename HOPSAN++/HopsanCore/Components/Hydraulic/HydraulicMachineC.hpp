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

#define pi 3.14159

#include <sstream>

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicMachineC : public ComponentC
    {

    private:
        double cp1, cp2, v1min, v2min;
        double alfa, wfak, betae, je, v1, v2, dp, cim, bm;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_t3, *mpND_a3, *mpND_w3,*mpND_c3, *mpND_Zx3, *mpND_eps;

        Delay mDelayedC1, mDelayedC2, mDelayedCp1, mDelayedCp2, mDelayedCp1e, mDelayedCp2e;
        Port *mpP1, *mpP2, *mpP3, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicMachineC("MachineC");
        }

        HydraulicMachineC(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            betae = 1000000000;
            je = 1;
            v1 = 0.005;
            v2 = 0.005;
            dp = 0.00005;
            cim = 0;
            bm = 0;
            alfa = 0.1;
            wfak = 0.1;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");
            mpIn = addReadPort("eps", "NodeSignal", Port::NOTREQUIRED);

            //Register changable parameters to the HOPSAN++ core
            registerParameter("Beta_e", "Bulk modulus of oil", "[Pa]", betae);
            registerParameter("J_e,m", "Equivalent load of inertia", "[kg*m^2]", je);
            registerParameter("V_1", "Volume at port 1", "[m^3]", v1);
            registerParameter("V_2", "Volume at port 2", "[m^3]", v2);
            registerParameter("D_m", "Displacement", "[m^3/rev]", dp);
            registerParameter("C_l,m", "Leakage coefficient", "[]", cim);
            registerParameter("B_m", "Viscous friction coefficient", "[Pa]", bm);

            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            //Assign node pointers
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

            mpND_eps = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 1.0);

            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, t3, a3, w3, c3, Zx3, eps;
            double dpr, dpe, ka, v1e, v2e, ap, wp, qp1, qp2;

            //Read input variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            t3 = (*mpND_t3);
            a3 = (*mpND_a3);
            w3 = (*mpND_w3);
            eps = (*mpND_eps);

            //Initialization
            dpr = dp / (pi * 2);
            dpe = dpr * eps;
            ka = 1 / (1 - alfa);
            v1min = 2*ka*betae * mTimestep*mTimestep * dpr*dpr / (2*wfak*je);
            v2min = 2*ka*betae * mTimestep*mTimestep * dpr*dpr / (2*wfak*je);
            v1e = v1;
            v2e = v2;
            if (v1e < v1min) { v1e = v1min; }
            if (v2e < v2min) { v2e = v2min; }
            ap = -a3;
            wp = -w3;
            Zc1 = 2 * ka*betae*mTimestep / (2*v1e);
            Zc2 = 2 * ka*betae*mTimestep / (2*v2e);
            mDelayedC1.initialize(1, p1-Zc1*q1);
            c1 = p1 - Zc1*q1;
            mDelayedC2.initialize(1, p2-Zc2*q2);
            c2 = p2 - Zc2*q2;
            qp1 = dpe * w3;
            qp2 = -dpe * w3;

            cp1 = p1 - Zc1 * (qp1 - cim * (p1 - p2));
            cp2 = p2 - Zc2 * (qp2 - cim * (p2 - p1));
            mDelayedCp1.initialize(1, cp1);
            mDelayedCp2.initialize(1, cp2);
            mDelayedCp1e.initialize(1, cp1);
            mDelayedCp2e.initialize(1, cp2);
            c3 = t3;
            Zx3 = dpe*dpe * Zc1 + dpe*dpe * Zc2 + bm;

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
            (*mpND_a3) = a3;
            (*mpND_w3) = w3;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare Local variables
            double cp10e, cp20e, ka, ap, c1e, c2e, p1e, p2e, ct1, ct2,
                   v1e, v2e, pm1, pp1, qp1, qp2, pp2, pm2, cp10, cp20, dpe, dpr, wp, ct1e,
                   ct2e, pm1e, pm2e, pp1e, qp1e, qp2e, pp2e, cp1e, cp2e;
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, t3, a3, w3, c3, Zx3, eps;

            //Read input variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);
            t3 = (*mpND_t3);
            a3 = (*mpND_a3);
            w3 = (*mpND_w3);
            eps = (*mpND_eps);

            //Machine equations
            v1e = std::max(v1,v1min);
            v2e = std::max(v2,v2min);

            std::stringstream ss;

            dpr = dp / (pi * 2);
            dpe = dpr * eps;    //Effective displacement
            ka = 1 / (1 - alfa);
            ap = -a3;
            wp = -w3;
            Zc1 = 2*ka*betae*mTimestep / (2*v1e);
            Zc2 = 2*ka*betae*mTimestep / (2*v2e);

            qp1 = dpe*w3;
            qp2 = -dpe*w3;

//            ss.flush();
//            ss << "cp1 = " << cp1;
//            this->addDebugMessage(ss.str());

            pp1 = (cp1 + qp1*Zc1 + cim*(cp2*Zc1 + cp1*Zc2)) / (cim*(Zc1 + Zc2) + 1);
            pp2 = (cp2 + qp2*Zc2 + cim*(cp2*Zc1 + cp1*Zc2)) / (cim*(Zc1 + Zc2) + 1);
            pp1e = pp1;
            pp2e = pp2;
            if (pp1e < 0.0) { pp1e = 0.0; }
            if (pp2e < 0.0) { pp2e = 0.0; }
            qp1e = qp1 - cim * (pp1e - pp2e);
            qp2e = qp2 - cim * (pp2e - pp1e);

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
            c3 = (cp1e - cp2e)*dpe;
            Zx3 = dpe*dpe * (Zc1 + Zc2) + bm;

            //Write output variables from nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
            (*mpND_t3) = t3;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;


//            ss.clear();
//            ss << "Output = " << c1 << " " << Zc1 << " " << c2 << " " << Zc2 << " " << t3 << " " << c3 << " " << Zx3;
//            this->addDebugMessage(ss.str());

        }
    };
}

#endif // HYDRAULICMACHINEC_HPP_INCLUDED
