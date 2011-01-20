//!
//! @file   HydraulicOpenCenterValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a hydraulic Open Center valve of Q-type
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
        double xv, xpanom, xpbnom, xatnom, xbtnom, xccnom, Kcpa, Kcpb, Kcat, Kcbt, Kccc, qpa, qpb, qat, qbt, qcc;

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb, *pmpND_c1, *qmpND_c1, *cmpND_c1, *ZcmpND_c1, *pmpND_c2, *qmpND_c2, *cmpND_c2, *ZcmpND_c2, *xvmpND_in;
        double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pc1, qc1, cc1, Zcc1, pc2, qc2, cc2, Zcc2;

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
            mTypeName = "HydraulicOpenCenterValve";
            Cq = 0.67;
            d = 0.01;
            f = 1.0;
            xvmax = 0.01;
            overlap_pa = 0.0;
            overlap_pb = 0.0;
            overlap_at = 0.0;
            overlap_bt = 0.0;
            overlap_cc = 0.0;
            omegah = 100.0;
            deltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpPC1 = addPowerPort("PC1", "NodeHydraulic");
            mpPC2 = addPowerPort("PC2", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", Cq);
            registerParameter("d", "Diameter", "[m]", d);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", f);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", xvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", overlap_pa);
            registerParameter("overlap_pb", "Spool Overlap From Port P To B", "[m]", overlap_pb);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", overlap_at);
            registerParameter("overlap_pa", "Spool Overlap From Port B To T", "[m]", overlap_bt);
            registerParameter("overlap_cc", "Spool Overlap From Port C1 To C2", "[m]", overlap_bt);
            registerParameter("omegah", "Resonance Frequency", "[rad/s]", omegah);
            registerParameter("deltah", "Damping Factor", "[-]", deltah);
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

            pmpND_c1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::PRESSURE);
            qmpND_c1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::FLOW);
            cmpND_c1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::WAVEVARIABLE);
            ZcmpND_c1 = getSafeNodeDataPtr(mpPC1, NodeHydraulic::CHARIMP);

            pmpND_c2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::PRESSURE);
            qmpND_c2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::FLOW);
            cmpND_c2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::WAVEVARIABLE);
            ZcmpND_c2 = getSafeNodeDataPtr(mpPC2, NodeHydraulic::CHARIMP);

            xvmpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);

            double xvin  = mpIn->readNode(NodeSignal::VALUE);
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            filter.initialize(mTimestep, num, den, xvin, xvin, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct  = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca  = (*mpND_ca);
            Zca = (*mpND_Zca);
            cb  = (*mpND_cb);
            Zcb = (*mpND_Zcb);
            cc1  = (*cmpND_c1);
            Zcc1 = (*ZcmpND_c1);
            cc2  = (*cmpND_c2);
            Zcc2 = (*ZcmpND_c2);
            xvin  = (*xvmpND_in);

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

            //Write new values to nodes

            (*mpND_pp) = cp + qp*Zcp;
            (*mpND_qp) = qp;
            (*mpND_pt) = ct + qt*Zct;
            (*mpND_qt) = qt;
            (*mpND_pa) = ca + qa*Zca;
            (*mpND_qa) = qa;
            (*mpND_pb) = cb + qb*Zcb;
            (*mpND_qb) = qb;
            (*pmpND_c1) = cc1 + qc1*Zcc1;
            (*qmpND_c1) = qc1;
            (*pmpND_c2) = cc2 + qc2*Zcc2;
            (*qmpND_c2) = qc2;
        }
    };
}

#endif // HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

