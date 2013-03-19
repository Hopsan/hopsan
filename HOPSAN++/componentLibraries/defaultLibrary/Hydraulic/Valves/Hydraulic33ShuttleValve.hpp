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
//! @file   Hydraulic33ShuttleValve.hpp
//! @author Bjorn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-08-18
//!
//! @brief Contains a hydraulic 3/3 shuttle valve of Q-type

#ifndef HYDRAULIC33SHUTTLEVALVE_HPP_INCLUDED
#define HYDRAULIC33SHUTTLEVALVE_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 3/3 shuttle valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic33ShuttleValve : public ComponentQ
    {
    private:
        double Cq;
        double d;
        double f_pa, f_at;
        double xvmax;
        double rho;
        double overlap_pa;
        double overlap_at;

        double d1, d2, K1, K2, A;

        double *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct;
        double *mpND_xvout;

        IntegratorLimited xIntegrator;

        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_at;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpX, *mpOut;

    public:
        static Component *Creator()
        {
            return new Hydraulic33ShuttleValve();
        }

        void configure()
        {
            Cq = 0.67;
            d = 0.01;
            f_pa = 1.0;
            f_at = 1.0;
            xvmax = 0.01;
            rho = 890;
            overlap_pa = -1e-6;
            overlap_at = -1e-6;

            d1 = 1e-3;
            d2 = 1e-3;
            A = pi*d*d/4.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpOut = addWritePort("xv_out", "NodeSignal", Port::NotRequired);

            registerParameter("C_q", "Flow Coefficient", "[-]", Cq);
            registerParameter("rho", "Oil Density", "[kg/m^3]", rho);
            registerParameter("d", "Spool Diameter", "[m]", d);
            registerParameter("f_pa", "Fraction of spool circumference that is opening P-A", "[-]", f_pa);
            registerParameter("f_at", "Fraction of spool circumference that is opening A-T", "[-]", f_at);
            registerParameter("x_vmax", "Maximum Spool Displacement", "[m]", xvmax);
            registerParameter("x_pa", "Spool Overlap From Port P To A", "[m]", overlap_pa);
            registerParameter("x_at", "Spool Overlap From Port A To T", "[m]", overlap_at);
            registerParameter("d_1", "Damp orifice 1 diam.", "[mm]", d1);
            registerParameter("d_2", "Damp orifice 2 diam.", "[mm]", d2);
        }


        void initialize()
        {
            mpND_pp = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpND_qp = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpND_cp = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpND_Zcp = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);

            mpND_pt = getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
            mpND_qt = getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
            mpND_ct = getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
            mpND_Zct = getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);

            mpND_pa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpND_qa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpND_ca = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpND_Zca = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            mpND_xvout = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            xIntegrator.initialize(mTimestep, 0, 0, -xvmax, xvmax);

            K1 = Cq*pi*d1*d1/4.0*sqrt(2.0/rho);
            K2 = Cq*pi*d2*d2/4.0*sqrt(2.0/rho);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;
            double pp, pt, pa, qa, ca, Zca, qp, cp, Zcp, qt, ct, Zct;
            double p, v;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca = (*mpND_ca);
            Zca = (*mpND_Zca);

            p = (K1*K1*(*mpND_pp) + K2*K2*(*mpND_pt))/(K1*K1+K2*K2);
            v = sign((*mpND_pp)-p)*K1*sqrt(fabs((*mpND_pp)-p))/A;
            xv = xIntegrator.update(v);

            //xv=-.01+mTime/10.0*0.02; //Test to see q(xv)

            xpanom = std::max(-xv-overlap_pa,0.0);
            xatnom = std::max(xv-overlap_at,0.0);

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_at.setFlowCoefficient(Kcat);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

            qp = -qpa;
            qa = qpa - qat;
            qt = qat;

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pp < 0.0)
            {
                cp = 0.0;
                Zcp = 0;
                cav = true;
            }
            if(pt < 0.0)
            {
                ct = 0.0;
                Zct = 0;
                cav = true;
            }

            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

                qp = -qpa;
                qa = qpa - qat;
                qt = qat;

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
            }

            //Write new values to nodes
            (*mpND_pp) = pp;
            (*mpND_qp) = qp;
            (*mpND_pa) = pa;
            (*mpND_qa) = qa;
            (*mpND_pt) = pt;
            (*mpND_qt) = qt;
            (*mpND_xvout) = xv;
        }
    };
}

#endif // HYDRAULIC33SHUTTLEVALVE_HPP_INCLUDED
