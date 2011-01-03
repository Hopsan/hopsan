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

        double *pp_ptr, *qp_ptr, *cp_ptr, *Zcp_ptr, *pt_ptr, *qt_ptr, *ct_ptr, *Zct_ptr, *pa_ptr, *qa_ptr, *ca_ptr, *Zca_ptr, *pb_ptr, *qb_ptr, *cb_ptr, *Zcb_ptr, *pc1_ptr, *qc1_ptr, *cc1_ptr, *Zcc1_ptr, *pc2_ptr, *qc2_ptr, *cc2_ptr, *Zcc2_ptr, *xvin_ptr;
        double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pc1, qc1, cc1, Zcc1, pc2, qc2, cc2, Zcc2;

        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        TurbulentFlowFunction mQturbpb;
        TurbulentFlowFunction mQturbat;
        TurbulentFlowFunction mQturbbt;
        TurbulentFlowFunction mQturbcc;
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
            pp_ptr = mpPP->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qp_ptr = mpPP->getNodeDataPtr(NodeHydraulic::FLOW);
            cp_ptr = mpPP->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zcp_ptr = mpPP->getNodeDataPtr(NodeHydraulic::CHARIMP);

            pt_ptr = mpPT->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qt_ptr = mpPT->getNodeDataPtr(NodeHydraulic::FLOW);
            ct_ptr = mpPT->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zct_ptr = mpPT->getNodeDataPtr(NodeHydraulic::CHARIMP);

            pa_ptr = mpPA->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qa_ptr = mpPA->getNodeDataPtr(NodeHydraulic::FLOW);
            ca_ptr = mpPA->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zca_ptr = mpPA->getNodeDataPtr(NodeHydraulic::CHARIMP);

            pb_ptr = mpPB->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qb_ptr = mpPB->getNodeDataPtr(NodeHydraulic::FLOW);
            cb_ptr = mpPB->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zcb_ptr = mpPB->getNodeDataPtr(NodeHydraulic::CHARIMP);

            pc1_ptr = mpPC1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qc1_ptr = mpPC1->getNodeDataPtr(NodeHydraulic::FLOW);
            cc1_ptr = mpPC1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zcc1_ptr = mpPC1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            pc2_ptr = mpPC2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qc2_ptr = mpPC2->getNodeDataPtr(NodeHydraulic::FLOW);
            cc2_ptr = mpPC2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zcc2_ptr = mpPC2->getNodeDataPtr(NodeHydraulic::CHARIMP);

            xvin_ptr = mpIn->getNodeDataPtr(NodeSignal::VALUE);

            double xvin  = mpIn->readNode(NodeSignal::VALUE);
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            myFilter.initialize(mTimestep, num, den, xvin, xvin, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            cp = (*cp_ptr);
            Zcp = (*Zcp_ptr);
            ct  = (*ct_ptr);
            Zct = (*Zct_ptr);
            ca  = (*ca_ptr);
            Zca = (*Zca_ptr);
            cb  = (*cb_ptr);
            Zcb = (*Zcb_ptr);
            cc1  = (*cc1_ptr);
            Zcc1 = (*Zcc1_ptr);
            cc2  = (*cc2_ptr);
            Zcc2 = (*Zcc2_ptr);
            xvin  = (*xvin_ptr);

            myFilter.update(xvin);
            xv = myFilter.value();

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
            mQturbpa.setFlowCoefficient(Kcpa);
            mQturbpb.setFlowCoefficient(Kcpb);
            mQturbat.setFlowCoefficient(Kcat);
            mQturbbt.setFlowCoefficient(Kcbt);
            mQturbcc.setFlowCoefficient(Kccc);

            qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            qpb = mQturbpb.getFlow(cp, cb, Zcp, Zcb);
            qat = mQturbat.getFlow(ca, ct, Zca, Zct);
            qbt = mQturbbt.getFlow(cb, ct, Zcb, Zct);
            qcc = mQturbcc.getFlow(cc1, cc2, Zcc1, Zcc2);

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

            (*pp_ptr) = cp + qp*Zcp;
            (*qp_ptr) = qp;
            (*pt_ptr) = ct + qt*Zct;
            (*qt_ptr) = qt;
            (*pa_ptr) = ca + qa*Zca;
            (*qa_ptr) = qa;
            (*pb_ptr) = cb + qb*Zcb;
            (*qb_ptr) = qb;
            (*pc1_ptr) = cc1 + qc1*Zcc1;
            (*qc1_ptr) = qc1;
            (*pc2_ptr) = cc2 + qc2*Zcc2;
            (*qc2_ptr) = qc2;
        }
    };
}

#endif // HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

