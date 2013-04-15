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
//! @file   HydraulicOpenCenterValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a hydraulic Open Center valve of Q-type
//$Id$

#ifndef HYDRAULICOPENCENTERVALVE_HPP_INCLUDED
#define HYDRAULICOPENCENTERVALVE_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic Open Center valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOpenCenterValve : public ComponentQ
    {
    private:
        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_pb, *mpF_at, *mpF_bt, *mpF_cc, *mpXvmax, *mpRho, *mpX_pa, *mpX_pb, *mpX_at, *mpX_bt, *mpX_cc;
        double omegah, deltah;

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb, *mpND_pc1, *mpND_qc1, *mpND_cc1, *mpND_Zcc1, *mpND_pc2, *mpND_qc2, *mpND_cc2, *mpND_Zcc2;

        SecondOrderTransferFunction filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_pb;
        TurbulentFlowFunction qTurb_at;
        TurbulentFlowFunction qTurb_bt;
        TurbulentFlowFunction qTurb_cc;
        Port *mpPP, *mpPC1, *mpPT, *mpPA, *mpPC2, *mpPB;

    public:
        static Component *Creator()
        {
            return new HydraulicOpenCenterValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpPC1 = addPowerPort("PC1", "NodeHydraulic");
            mpPC2 = addPowerPort("PC2", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);
            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);
            addInputVariable("C_q", "Flow Coefficient", "[-]", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "[kg/m^3]", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "[m]", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "[-]", 1.0, &mpF_pa);
            addInputVariable("f_pb", "Fraction of spool circumference that is opening B-T", "[-]", 1.0, &mpF_pb);
            addInputVariable("f_at", "Fraction of spool circumference that is opening P-A", "[-]", 1.0, &mpF_at);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "[-]", 1.0, &mpF_bt);
            addInputVariable("f_cc", "Fraction of spool circumference that is opening C-C", "[-]", 1.0, &mpF_cc);
            addInputVariable("x_pa", "Spool Overlap From Port P To A", "[m]", -1e-6, &mpX_pa);
            addInputVariable("x_pb", "Spool Overlap From Port A To T", "[m]", -1e-6, &mpX_pb);
            addInputVariable("x_at", "Spool Overlap From Port P To A", "[m]", -1e-6, &mpX_at);
            addInputVariable("x_bt", "Spool Overlap From Port A To T", "[m]", -1e-6, &mpX_bt);
            addInputVariable("x_cc", "Spool Overlap From Port C1 To C2", "[m]", -1e-6, &mpX_cc);
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

            mpND_pc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::Pressure);
            mpND_qc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::Flow);
            mpND_cc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::WaveVariable);
            mpND_Zcc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::CharImpedance);

            mpND_pc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::Pressure);
            mpND_qc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::Flow);
            mpND_cc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::WaveVariable);
            mpND_Zcc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::CharImpedance);

            double xvin  = (*mpXvIn);
            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            filter.initialize(mTimestep, num, den, (*mpXvIn), (*mpXvIn), -(*mpXvmax), (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xpbnom, xatnom, xbtnom, xccnom, Kcpa, Kcpb, Kcat, Kcbt, Kccc, qpa, qpb, qat, qbt, qcc;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pc1, qc1, cc1, Zcc1, pc2, qc2, cc2, Zcc2;
            double Cq, rho, xvmax, d, f_pa, f_pb, f_at, f_bt, f_cc, x_pa, x_pb, x_at, x_bt, x_cc;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct  = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca  = (*mpND_ca);
            Zca = (*mpND_Zca);
            cb  = (*mpND_cb);
            Zcb = (*mpND_Zcb);
            cc1  = (*mpND_cc1);
            Zcc1 = (*mpND_Zcc1);
            cc2  = (*mpND_cc2);
            Zcc2 = (*mpND_Zcc2);

            xvin  = (*mpXvIn);
            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);
            f_bt = (*mpF_bt);
            f_cc = (*mpF_cc);
            x_pa = (*mpX_pa);
            x_pb = (*mpX_pb);
            x_at = (*mpX_at);
            x_bt = (*mpX_bt);
            x_cc = (*mpX_cc);

            limitValue(xvin, -xvmax, xvmax);
            filter.update(xvin);
            xv = filter.value();

            //Valve equations
            xpanom = std::max(xv-x_pa,0.0);                       //These orifices are closed in central position, and fully opened at -xvmax and xvmax
            xpbnom = std::max(-xv-x_pb,0.0);
            xatnom = std::max(-xv-x_at,0.0);
            xbtnom = std::max(xv-x_bt,0.0);
            xccnom = xvmax - std::max(fabs(xv-x_cc), 0.0);       //Center orifice is open in central position, and closed at -xvmax and xvmax

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcpb = Cq*f_pb*pi*d*xpbnom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xbtnom*sqrt(2.0/rho);
            Kccc = Cq*f_cc*pi*d*xccnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_pb.setFlowCoefficient(Kcpb);
            qTurb_at.setFlowCoefficient(Kcat);
            qTurb_bt.setFlowCoefficient(Kcbt);
            qTurb_cc.setFlowCoefficient(Kccc);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);
            qcc = qTurb_cc.getFlow(cc1, cc2, Zcc1, Zcc2);

            qc1 = -qcc;
            qc2 = qcc;
            if (xv >= 0.0)
            {
                qp = -qpa;
                qa = qpa;
                qb = -qbt;
                qt = qbt;
            }
            else
            {
                qp = -qpb;
                qa = -qat;
                qb = qpb;
                qt = qat;
            }

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;
            pb = cb + qb*Zcb;
            pc1 = cc1 + qc1*Zcc1;
            pc2 = cc2 + qc2*Zcc2;

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
            if(pc1 < 0.0)
            {
                cc1 = 0.0;
                Zcc1 = 0;
                cav = true;
            }
            if(pc2 < 0.0)
            {
                cc2 = 0.0;
                Zcc2 = 0;
                cav = true;
            }

            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
                qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);
                qcc = qTurb_cc.getFlow(cc1, cc2, Zcc1, Zcc2);

                if (xv >= 0.0)
                {
                    qp = -qpa;
                    qa = qpa;
                    qb = -qbt;
                    qt = qbt;
                }
                else
                {
                    qp = -qpb;
                    qa = -qat;
                    qb = qpb;
                    qt = qat;
                }

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
                pc1 = cc1 + qc1*Zcc1;
                pc2 = cc2 + qc2*Zcc2;
            }

            //Write new values to nodes

            (*mpND_pp) = cp + qp*Zcp;
            (*mpND_qp) = qp;
            (*mpND_pt) = ct + qt*Zct;
            (*mpND_qt) = qt;
            (*mpND_pa) = ca + qa*Zca;
            (*mpND_qa) = qa;
            (*mpND_pb) = cb + qb*Zcb;
            (*mpND_qb) = qb;
            (*mpND_pc1) = cc1 + qc1*Zcc1;
            (*mpND_qc1) = qc1;
            (*mpND_pc2) = cc2 + qc2*Zcc2;
            (*mpND_qc2) = qc2;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

