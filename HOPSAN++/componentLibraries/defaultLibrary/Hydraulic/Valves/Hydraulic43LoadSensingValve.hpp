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
//! @file   Hydraulic43LoadSensingValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hydraulic 4/3-valve with load sensing port of Q-type
#ifndef HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED
#define HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/3-valve (closed centre) with load sensing port of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic43LoadSensingValve : public ComponentQ
    {
    private:
        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_pb, *mpF_at, *mpF_bt, *mpXvmax, *mpRho, *mpX_pa, *mpX_pb, *mpX_at, *mpX_bt;
        double omegah, deltah;

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb, *mpND_pload, *mpND_qload, *mpND_cload, *mpND_Zcload;

        SecondOrderTransferFunction filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_pb;
        TurbulentFlowFunction qTurb_at;
        TurbulentFlowFunction qTurb_bt;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpPL;

    public:
        static Component *Creator()
        {
            return new Hydraulic43LoadSensingValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpPL = addPowerPort("PL", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);
            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);
            addInputVariable("C_q", "Flow Coefficient", "[-]", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "[kg/m^3]", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "[m]", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "[-]", 1.0, &mpF_pa);
            addInputVariable("f_pb", "Fraction of spool circumference that is opening B-T", "[-]", 1.0, &mpF_pb);
            addInputVariable("f_at", "Fraction of spool circumference that is opening P-A", "[-]", 1.0, &mpF_at);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "[-]", 1.0, &mpF_bt);
            addInputVariable("x_pa", "Spool Overlap From Port P To A", "[m]", -1e-6, &mpX_pa);
            addInputVariable("x_pb", "Spool Overlap From Port A To T", "[m]", -1e-6, &mpX_pb);
            addInputVariable("x_at", "Spool Overlap From Port P To A", "[m]", -1e-6, &mpX_at);
            addInputVariable("x_bt", "Spool Overlap From Port A To T", "[m]", -1e-6, &mpX_bt);
            addInputVariable("x_vmax", "Maximum Spool Displacement", "[m]", 0.01, &mpXvmax);

            addConstant("omega_h", "Resonance Frequency", "[rad/s]", 100.0, omegah);
            addConstant("delta_h", "Damping Factor", "[-]", 1.0, deltah);
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

            mpND_pb = getSafeNodeDataPtr(mpPB, NodeHydraulic::Pressure);
            mpND_qb = getSafeNodeDataPtr(mpPB, NodeHydraulic::Flow);
            mpND_cb = getSafeNodeDataPtr(mpPB, NodeHydraulic::WaveVariable);
            mpND_Zcb = getSafeNodeDataPtr(mpPB, NodeHydraulic::CharImpedance);

            mpND_pload = getSafeNodeDataPtr(mpPL, NodeHydraulic::Pressure);
            mpND_qload = getSafeNodeDataPtr(mpPL, NodeHydraulic::Flow);
            mpND_cload = getSafeNodeDataPtr(mpPL, NodeHydraulic::WaveVariable);
            mpND_Zcload = getSafeNodeDataPtr(mpPL, NodeHydraulic::CharImpedance);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            filter.initialize(mTimestep, num, den, 0, 0, -(*mpXvmax), (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt;
            double Cq, rho, xvmax, d, f_pa, f_pb, f_at, f_bt, x_pa, x_pb, x_at, x_bt;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pload, cload, Zcload;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca = (*mpND_ca);
            Zca = (*mpND_Zca);
            cb = (*mpND_cb);
            Zcb = (*mpND_Zcb);
            cload = (*mpND_cload);
            Zcload = (*mpND_Zcload);
            xvin = (*mpXvIn);

            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);
            f_bt = (*mpF_bt);
            x_pa = (*mpX_pa);
            x_pb = (*mpX_pb);
            x_at = (*mpX_at);
            x_bt = (*mpX_bt);

            limitValue(xvin, -xvmax, xvmax);
            filter.update(xvin);
            xv = filter.value();

            xpanom = std::max(xv-x_pa,0.0);
            xpbnom = std::max(-xv-x_pb,0.0);
            xatnom = std::max(-xv-x_at,0.0);
            xbtnom = std::max(xv-x_bt,0.0);

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcpb = Cq*f_pb*pi*d*xpbnom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xbtnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_pb.setFlowCoefficient(Kcpb);
            qTurb_at.setFlowCoefficient(Kcat);
            qTurb_bt.setFlowCoefficient(Kcbt);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);

            qp = -qpa-qpb;
            qa = qpa-qat;
            qb = -qbt+qpb;
            qt = qat+qbt;

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;
            pb = cb + qb*Zcb;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pb < 0.0)
            {
                cb = 0.0;
                Zcb = 0;
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
                qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
                qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);

                qp = -qpa-qpb;
                qa = qpa-qat;
                qb = -qbt+qpb;
                qt = qat+qbt;

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
            }


            if(xv >= 0.0)
            {
                pload = pa;
            }
            else
            {
                pload = pb;
            }

            //Write new values to nodes

            (*mpND_pp) = pp;
            (*mpND_qp) = qp;
            (*mpND_pt) = pt;
            (*mpND_qt) = qt;
            (*mpND_pa) = pa;
            (*mpND_qa) = qa;
            (*mpND_pb) = pb;
            (*mpND_qb) = qb;
            (*mpND_pload) = pload;
            (*mpND_qload) = (pload - cload)/Zcload;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

