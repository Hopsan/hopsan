//!
//! @file   Hydraulic43LoadSensingValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hydraulic 4/3-valve with load sensing port of Q-type
#ifndef HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED
#define HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/3-valve (closed centre) with load sensing port of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic43LoadSensingValve : public ComponentQ
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

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb, *mpND_pload, *mpND_qload, *mpND_cload, *mpND_Zcload, *mpND_xvin;

        SecondOrderFilter filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_pb;
        TurbulentFlowFunction qTurb_at;
        TurbulentFlowFunction qTurb_bt;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn, *mpPL;

    public:
        static Component *Creator()
        {
            return new Hydraulic43LoadSensingValve("Hydraulic 4/3 Valve with Load Sensing Port");
        }

        Hydraulic43LoadSensingValve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic43LoadSensingValve";
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
            mpPL = addPowerPort("PL", "NodeHydraulic");
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

            mpND_pload = getSafeNodeDataPtr(mpPL, NodeHydraulic::PRESSURE);
            mpND_qload = getSafeNodeDataPtr(mpPL, NodeHydraulic::FLOW);
            mpND_cload = getSafeNodeDataPtr(mpPL, NodeHydraulic::WAVEVARIABLE);
            mpND_Zcload = getSafeNodeDataPtr(mpPL, NodeHydraulic::CHARIMP);

            mpND_xvin = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);

            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            filter.initialize(mTimestep, num, den, 0, 0, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pload, qload, cload, Zcload;
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
            xvin = (*mpND_xvin);

            filter.update(xvin);
            xv = filter.value();

            xpanom = std::max(xv-overlap_pa,0.0);
            xpbnom = std::max(-xv-overlap_pb,0.0);
            xatnom = std::max(-xv-overlap_at,0.0);
            xbtnom = std::max(xv-overlap_bt,0.0);

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/890.0);
            Kcpb = Cq*f*pi*d*xpbnom*sqrt(2.0/890.0);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/890.0);
            Kcbt = Cq*f*pi*d*xbtnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_pb.setFlowCoefficient(Kcpb);
            qTurb_at.setFlowCoefficient(Kcat);
            qTurb_bt.setFlowCoefficient(Kcbt);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);

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
        }
    };
}

#endif // HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

