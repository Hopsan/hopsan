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
//! @file   Hydraulic22PoppetValve.hpp
//! @author Bjorn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-08-26
//!
//! @brief Contains a hydraulic 2/2 poppet valve of Q-type

#ifndef HYDRAULIC22POPPET_HPP_INCLUDED
#define HYDRAULIC22POPPET_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 2/2 poppet of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22PoppetValve : public ComponentQ
    {
    private:
        double Cq;
        double frac_d;
        double xvmax;
        double rho;

        double d1, d2, dd, A1, A2, A3;
        double K21, K3, k, F0;
        double v;

        double *mpND_pA1, *mpND_qA1, *mpND_cA1, *mpND_ZcA1,
               *mpND_pA2, *mpND_qA2, *mpND_cA2, *mpND_ZcA2,
               *mpND_pA3, *mpND_qA3, *mpND_cA3, *mpND_ZcA3;
        double *mpND_xvout;

        IntegratorLimited xIntegrator;

        TurbulentFlowFunction qTurb_pA2A1;
        TurbulentFlowFunction qTurb_A3;
        Port *mpA1, *mpA2, *mpA3, *mpOut;

    public:
        static Component *Creator()
        {
            return new Hydraulic22PoppetValve();
        }

        Hydraulic22PoppetValve() : ComponentQ()
        {
            Cq = 0.67;
            frac_d = 1.0;
            xvmax = 0.01;
            rho = 890;

            d1 = 10e-3;
            d2 = 15e-3;
            dd = 0.1e-3;
            A1 = pi*d1*d1/4.0;
            A3 = pi*d2*d2/4.0;
            A2 = A3 - A1;
            k = 1e4;
            F0 = 100;

            mpA1 = addPowerPort("PN", "NodeHydraulic");
            mpA2 = addPowerPort("PS", "NodeHydraulic");
            mpA3 = addPowerPort("PC", "NodeHydraulic");
            mpOut = addWritePort("xv_out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("C_q", "Flow Coefficient", "[-]", Cq);
            registerParameter("rho", "Oil Density", "[kg/m^3]", rho);
            registerParameter("d_1", "Small diameter", "[m]", d1);
            registerParameter("d_2", "Big diameter", "[m]", d2);
            registerParameter("k", "Spring constant", "[N/m]", k);
            registerParameter("F_0", "Spring pre-load", "[N]", F0);
            registerParameter("f", "Fraction of poppet diameter that is opening", "[-]", frac_d);
            registerParameter("x_v,max", "Maximum Spool Displacement", "[m]", xvmax);
            registerParameter("d_d", "Damp orifice diam.", "[m]", dd);
        }


        void initialize()
        {
            mpND_pA1 = getSafeNodeDataPtr(mpA1, NodeHydraulic::PRESSURE);
            mpND_qA1 = getSafeNodeDataPtr(mpA1, NodeHydraulic::FLOW);
            mpND_cA1 = getSafeNodeDataPtr(mpA1, NodeHydraulic::WAVEVARIABLE);
            mpND_ZcA1 = getSafeNodeDataPtr(mpA1, NodeHydraulic::CHARIMP);

            mpND_pA2 = getSafeNodeDataPtr(mpA2, NodeHydraulic::PRESSURE);
            mpND_qA2 = getSafeNodeDataPtr(mpA2, NodeHydraulic::FLOW);
            mpND_cA2 = getSafeNodeDataPtr(mpA2, NodeHydraulic::WAVEVARIABLE);
            mpND_ZcA2 = getSafeNodeDataPtr(mpA2, NodeHydraulic::CHARIMP);

            mpND_pA3 = getSafeNodeDataPtr(mpA3, NodeHydraulic::PRESSURE);
            mpND_qA3 = getSafeNodeDataPtr(mpA3, NodeHydraulic::FLOW);
            mpND_cA3 = getSafeNodeDataPtr(mpA3, NodeHydraulic::WAVEVARIABLE);
            mpND_ZcA3 = getSafeNodeDataPtr(mpA3, NodeHydraulic::CHARIMP);

            mpND_xvout = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            xIntegrator.initialize(mTimestep, 0.0, 0.0, 0.0, xvmax);

            K21 = Cq*pi*d1*frac_d*0.0*sqrt(2.0/rho);
            K3  = Cq*pi*dd*dd/4.0*sqrt(2.0/rho);
            qTurb_A3.setFlowCoefficient(K3);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cA1, ZcA1, pA1, qA1,
                   cA2, ZcA2, pA2, qA2,
                   cA3, ZcA3, pA3, qA3;
            double pc;
            bool cav = false;

            //Get variable values from nodes
            cA1  = (*mpND_cA1);
            ZcA1 = (*mpND_ZcA1);
            cA2  = (*mpND_cA2);
            ZcA2 = (*mpND_ZcA2);
            cA3  = (*mpND_cA3);
            ZcA3 = (*mpND_ZcA3);

            K21 = Cq*pi*d1*frac_d*xIntegrator.value()*sqrt(2.0/rho);
            qTurb_pA2A1.setFlowCoefficient(K21);

            qA1 = qTurb_pA2A1.getFlow(cA2, cA1, ZcA1, ZcA2);
            qA2 = -qA1;

            pA1 = cA1 + ZcA1*qA1;
            pA2 = cA2 + ZcA2*qA2;

            pc = (A1*pA1 + A2*pA2 -(F0+k*xIntegrator.value()))/A3;

            qA3 = qTurb_A3.getFlow(pc, cA3, 0.0, ZcA3);
            v = qA3/A3;

            pA3 = cA3 + ZcA3*qA3;

            //Cavitation check
            if(pA1 < 0.0)
            {
                cA1  = 0.0;
                ZcA1 = 0.0;
                cav  = true;
            }
            if(pA2 < 0.0)
            {
                cA2  = 0.0;
                ZcA2 = 0.0;
                cav  = true;
            }
            if(pA3 < 0.0)
            {
                cA3  = 0.0;
                ZcA3 = 0.0;
                cav  = true;
            }

            if(cav)
            {
                qA1 = qTurb_pA2A1.getFlow(cA2, cA1, ZcA1, ZcA2);
                qA2 = -qA1;

                pA1 = cA1 + ZcA1*qA1;
                pA2 = cA2 + ZcA2*qA2;

                pc = (A1*pA1 + A2*pA2 -(F0+k*xIntegrator.value()))/A3;

                v = qTurb_A3.getFlow(pc, cA3, 0.0, ZcA3)/A3;
            }

            xIntegrator.update(v);

            //Write new values to nodes
            (*mpND_pA1) = pA1;
            (*mpND_qA1) = qA1;
            (*mpND_pA2) = pA2;
            (*mpND_qA2) = qA2;
            (*mpND_pA3) = pA3;
            (*mpND_qA3) = qA3;
            (*mpND_xvout) = xIntegrator.value();
        }
    };
}

#endif // HYDRAULIC22POPPETVALVE_HPP_INCLUDED
