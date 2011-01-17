//!
//! @file   Hydraulic43Valve.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hydraulic 4/3-valve of Q-type
#ifndef HYDRAULIC43VALVE_HPP_INCLUDED
#define HYDRAULIC43VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/3-valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic43Valve : public ComponentQ
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
        double omegah;
        double deltah;

        double cp, Zcp, ct, Zct, ca, Zca, cb, Zcb, xvin, xv, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt, qp, qa, qb, qt, pa, pb, pt, pp;

        double *pp_ptr, *qp_ptr, *pt_ptr, *qt_ptr, *pa_ptr, *qa_ptr, *pb_ptr, *qb_ptr;
        double *cp_ptr, *Zcp_ptr, *ct_ptr, *Zct_ptr, *ca_ptr, *Zca_ptr, *cb_ptr, *Zcb_ptr, *xvmpND_in;

        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        TurbulentFlowFunction mQturbpb;
        TurbulentFlowFunction mQturbat;
        TurbulentFlowFunction mQturbbt;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic43Valve("Hydraulic 4/3 Valve");
        }

        Hydraulic43Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic43Valve";
            Cq = 0.67;
            d = 0.01;
            f = 1.0;
            xvmax = 0.01;
            overlap_pa = 0.0;
            overlap_pb = 0.0;
            overlap_at = 0.0;
            overlap_bt = 0.0;
            omegah = 100.0;
            deltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", Cq);
            registerParameter("d", "Diameter", "[m]", d);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", f);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", xvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", overlap_pa);
            registerParameter("overlap_pb", "Spool Overlap From Port P To B", "[m]", overlap_pb);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", overlap_at);
            registerParameter("overlap_pa", "Spool Overlap From Port B To T", "[m]", overlap_bt);
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

            xvmpND_in = mpIn->getNodeDataPtr(NodeSignal::VALUE);

            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            myFilter.initialize(mTimestep, num, den, 0, 0, -xvmax, xvmax);
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
            xvin  = (*xvmpND_in);

            myFilter.update(xvin);
            xv = myFilter.value();

            //Valve equations
            xpanom = std::max(xv-overlap_pa,0.0);
            xpbnom = std::max(-xv-overlap_pb,0.0);
            xatnom = std::max(-xv-overlap_at,0.0);
            xbtnom = std::max(xv-overlap_bt,0.0);

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/890.0);
            Kcpb = Cq*f*pi*d*xpbnom*sqrt(2.0/890.0);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/890.0);
            Kcbt = Cq*f*pi*d*xbtnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            mQturbpa.setFlowCoefficient(Kcpa);
            mQturbpb.setFlowCoefficient(Kcpb);
            mQturbat.setFlowCoefficient(Kcat);
            mQturbbt.setFlowCoefficient(Kcbt);

            qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            qpb = mQturbpb.getFlow(cp, cb, Zcp, Zcb);
            qat = mQturbat.getFlow(ca, ct, Zca, Zct);
            qbt = mQturbbt.getFlow(cb, ct, Zcb, Zct);

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
        }
    };
}

#endif // HYDRAULIC43VALVE_HPP_INCLUDED

