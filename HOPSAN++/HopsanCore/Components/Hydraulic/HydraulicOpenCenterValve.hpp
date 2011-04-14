//!
//! @file   HydraulicOpenCenterValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a hydraulic Open Center valve of Q-type
//$Id$

#ifndef HYDRAULICOPENCENTERVALVE_HPP_INCLUDED
#define HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic Open Center valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOpenCenterValve : public ComponentQ
    {
    private:
        double Cq;
        double d;
        double f;
        double xvmax;
        double overlap_pa;
        double overlap_pb;
        double overlap_at;
        double overlap_bt;
        double overlap_cc;
        double omegah;
        double deltah;

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb, *mpND_pc1, *mpND_qc1, *mpND_cc1, *mpND_Zcc1, *mpND_pc2, *mpND_qc2, *mpND_cc2, *mpND_Zcc2, *mpND_xvin;

        SecondOrderFilter filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_pb;
        TurbulentFlowFunction qTurb_at;
        TurbulentFlowFunction qTurb_bt;
        TurbulentFlowFunction qTurb_cc;
        Port *mpPP, *mpPC1, *mpPT, *mpPA, *mpPC2, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicOpenCenterValve("Hydraulic 5/3 Valve");
        }

        HydraulicOpenCenterValve(const std::string name) : ComponentQ(name)
        {
            Cq = 0.67;
            d = 0.01;
            f = 1.0;
            xvmax = 0.01;
            overlap_pa = -1e-6;
            overlap_pb = -1e-6;
            overlap_at = -1e-6;
            overlap_bt = -1e-6;
            overlap_cc = -1e-6;
            omegah = 100.0;
            deltah = 1.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpPC1 = addPowerPort("PC1", "NodeHydraulic");
            mpPC2 = addPowerPort("PC2", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("C_q", "Flow Coefficient", "[-]", Cq);
            registerParameter("d", "Diameter", "[m]", d);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", f);
            registerParameter("x_v,max", "Maximum Spool Displacement", "[m]", xvmax);
            registerParameter("x_pa", "Spool Overlap From Port P To A", "[m]", overlap_pa);
            registerParameter("x_pb", "Spool Overlap From Port P To B", "[m]", overlap_pb);
            registerParameter("x_at", "Spool Overlap From Port A To T", "[m]", overlap_at);
            registerParameter("x_pa", "Spool Overlap From Port B To T", "[m]", overlap_bt);
            registerParameter("x_cc", "Spool Overlap From Port C1 To C2", "[m]", overlap_bt);
            registerParameter("omega_h", "Resonance Frequency", "[rad/s]", omegah);
            registerParameter("delta_h", "Damping Factor", "[-]", deltah);
        }


        void initialize()
        {
            mpND_pp = getSafeNodeDataPtr(mpPP, NodeHydraulic::PRESSURE);
            mpND_qp = getSafeNodeDataPtr(mpPP, NodeHydraulic::FLOW);
            mpND_cp = getSafeNodeDataPtr(mpPP, NodeHydraulic::WAVEVARIABLE);
            mpND_Zcp = getSafeNodeDataPtr(mpPP, NodeHydraulic::CHARIMP);

            mpND_pt = getSafeNodeDataPtr(mpPT, NodeHydraulic::PRESSURE);
            mpND_qt = getSafeNodeDataPtr(mpPT, NodeHydraulic::FLOW);
            mpND_ct = getSafeNodeDataPtr(mpPT, NodeHydraulic::WAVEVARIABLE);
            mpND_Zct = getSafeNodeDataPtr(mpPT, NodeHydraulic::CHARIMP);

            mpND_pa = getSafeNodeDataPtr(mpPA, NodeHydraulic::PRESSURE);
            mpND_qa = getSafeNodeDataPtr(mpPA, NodeHydraulic::FLOW);
            mpND_ca = getSafeNodeDataPtr(mpPA, NodeHydraulic::WAVEVARIABLE);
            mpND_Zca = getSafeNodeDataPtr(mpPA, NodeHydraulic::CHARIMP);

            mpND_pb = getSafeNodeDataPtr(mpPB, NodeHydraulic::PRESSURE);
            mpND_qb = getSafeNodeDataPtr(mpPB, NodeHydraulic::FLOW);
            mpND_cb = getSafeNodeDataPtr(mpPB, NodeHydraulic::WAVEVARIABLE);
            mpND_Zcb = getSafeNodeDataPtr(mpPB, NodeHydraulic::CHARIMP);

            mpND_pc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::PRESSURE);
            mpND_qc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::FLOW);
            mpND_cc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zcc1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::CHARIMP);

            mpND_pc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::PRESSURE);
            mpND_qc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::FLOW);
            mpND_cc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zcc2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::CHARIMP);

            mpND_xvin = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);

            double xvin  = mpIn->readNode(NodeSignal::VALUE);
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            filter.initialize(mTimestep, num, den, xvin, xvin, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xpbnom, xatnom, xbtnom, xccnom, Kcpa, Kcpb, Kcat, Kcbt, Kccc, qpa, qpb, qat, qbt, qcc;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pc1, qc1, cc1, Zcc1, pc2, qc2, cc2, Zcc2;
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
            xvin  = (*mpND_xvin);

            filter.update(xvin);
            xv = filter.value();

            //Valve equations
            xpanom = std::max(xv-overlap_pa,0.0);                       //These orifices are closed in central position, and fully opened at -xvmax and xvmax
            xpbnom = std::max(-xv-overlap_pb,0.0);
            xatnom = std::max(-xv-overlap_at,0.0);
            xbtnom = std::max(xv-overlap_bt,0.0);
            xccnom = xvmax - std::max(fabs(xv-overlap_cc), 0.0);       //Center orifice is open in central position, and closed at -xvmax and xvmax

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/890.0);
            Kcpb = Cq*f*pi*d*xpbnom*sqrt(2.0/890.0);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/890.0);
            Kcbt = Cq*f*pi*d*xbtnom*sqrt(2.0/890.0);
            Kccc = Cq*f*pi*d*xccnom*sqrt(2.0/890.0);

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
        }
    };
}

#endif // HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

