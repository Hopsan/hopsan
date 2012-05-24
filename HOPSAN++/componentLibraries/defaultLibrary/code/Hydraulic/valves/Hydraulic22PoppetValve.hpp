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

        double d1, d2, dd, AN, AS, AC;
        double K21, K3, k, F0;
        double v;

        double *mpND_pAN, *mpND_qAN, *mpND_cAN, *mpND_ZcAN,
               *mpND_pAS, *mpND_qAS, *mpND_cAS, *mpND_ZcAS,
               *mpND_pAC, *mpND_qAC, *mpND_cAC, *mpND_ZcAC;
        double *mpND_xvout;

        IntegratorLimited xIntegrator;

        TurbulentFlowFunction qTurb_pASAN;
        TurbulentFlowFunction qTurb_AC;
        Port *mpAN, *mpAS, *mpAC, *mpOut;

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
            k = 1e4;
            F0 = 100;

            mpAN = addPowerPort("PN", "NodeHydraulic");
            mpAS = addPowerPort("PS", "NodeHydraulic");
            mpAC = addPowerPort("PC", "NodeHydraulic");
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
            AN = pi*d1*d1/4.0;
            AC = pi*d2*d2/4.0;
            AS = AC - AN;

            mpND_pAN = getSafeNodeDataPtr(mpAN, NodeHydraulic::PRESSURE);
            mpND_qAN = getSafeNodeDataPtr(mpAN, NodeHydraulic::FLOW);
            mpND_cAN = getSafeNodeDataPtr(mpAN, NodeHydraulic::WAVEVARIABLE);
            mpND_ZcAN = getSafeNodeDataPtr(mpAN, NodeHydraulic::CHARIMP);

            mpND_pAS = getSafeNodeDataPtr(mpAS, NodeHydraulic::PRESSURE);
            mpND_qAS = getSafeNodeDataPtr(mpAS, NodeHydraulic::FLOW);
            mpND_cAS = getSafeNodeDataPtr(mpAS, NodeHydraulic::WAVEVARIABLE);
            mpND_ZcAS = getSafeNodeDataPtr(mpAS, NodeHydraulic::CHARIMP);

            mpND_pAC = getSafeNodeDataPtr(mpAC, NodeHydraulic::PRESSURE);
            mpND_qAC = getSafeNodeDataPtr(mpAC, NodeHydraulic::FLOW);
            mpND_cAC = getSafeNodeDataPtr(mpAC, NodeHydraulic::WAVEVARIABLE);
            mpND_ZcAC = getSafeNodeDataPtr(mpAC, NodeHydraulic::CHARIMP);

            mpND_xvout = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            xIntegrator.initialize(mTimestep, 0.0, 0.0, 0.0, xvmax);

            K21 = Cq*pi*d1*frac_d*0.0*sqrt(2.0/rho); //Main flow coeff.
            K3  = Cq*pi*dd*dd/4.0*sqrt(2.0/rho); //Damping orifice flow coeff.
            qTurb_AC.setFlowCoefficient(K3);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cAN, ZcAN, pAN, qAN,
                   cAS, ZcAS, pAS, qAS,
                   cAC, ZcAC, pAC, qAC;
            double pc;
            bool cav = false;

            //Get variable values from nodes
            cAN  = (*mpND_cAN);
            ZcAN = (*mpND_ZcAN);
            cAS  = (*mpND_cAS);
            ZcAS = (*mpND_ZcAS);
            cAC  = (*mpND_cAC);
            ZcAC = (*mpND_ZcAC);

            K21 = Cq*pi*d1*frac_d*xIntegrator.value()*sqrt(2.0/rho);
            qTurb_pASAN.setFlowCoefficient(K21);

            qAN = qTurb_pASAN.getFlow(cAS, cAN, ZcAN, ZcAS);
            qAS = -qAN;

            pAN = cAN + ZcAN*qAN;
            pAS = cAS + ZcAS*qAS;

            pc = (AN*pAN + AS*pAS -(F0+k*xIntegrator.value()))/AC;

            qAC = qTurb_AC.getFlow(pc, cAC, 0.0, ZcAC);
            v = qAC/AC;

            pAC = cAC + ZcAC*qAC;

            //Cavitation check
            if(pAN < 0.0)
            {
                cAN  = 0.0;
                ZcAN = 0.0;
                cav  = true;
            }
            if(pAS < 0.0)
            {
                cAS  = 0.0;
                ZcAS = 0.0;
                cav  = true;
            }
            if(pAC < 0.0)
            {
                cAC  = 0.0;
                ZcAC = 0.0;
                cav  = true;
            }

            if(cav)
            {
                qAN = qTurb_pASAN.getFlow(cAS, cAN, ZcAN, ZcAS);
                qAS = -qAN;

                pAN = cAN + ZcAN*qAN;
                pAS = cAS + ZcAS*qAS;

                pc = (AN*pAN + AS*pAS -(F0+k*xIntegrator.value()))/AC;

                v = qTurb_AC.getFlow(pc, cAC, 0.0, ZcAC)/AC;
            }

            xIntegrator.update(v);

            //Write new values to nodes
            (*mpND_pAN) = pAN;
            (*mpND_qAN) = qAN;
            (*mpND_pAS) = pAS;
            (*mpND_qAS) = qAS;
            (*mpND_pAC) = pAC;
            (*mpND_qAC) = qAC;
            (*mpND_xvout) = xIntegrator.value();
        }
    };
}

#endif // HYDRAULIC22POPPETVALVE_HPP_INCLUDED
