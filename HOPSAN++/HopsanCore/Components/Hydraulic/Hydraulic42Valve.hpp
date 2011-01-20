//!
//! @file   Hydraulic42Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 4/2-valve of Q-type

#ifndef HYDRAULIC42VALVE_HPP_INCLUDED
#define HYDRAULIC42VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic42Valve : public ComponentQ
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

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb, *xvmpND_in;

        SecondOrderFilter filter;
        TurbulentFlowFunction qTurb__pa;
        TurbulentFlowFunction qTurb__bt;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic42Valve("Hydraulic 4/2 Valve");
        }

        Hydraulic42Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic42Valve";
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
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", overlap_pa);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", overlap_pb);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", overlap_at);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", overlap_bt);
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

            xvmpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);

            //Initiate second order low pass filter
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            filter.initialize(mTimestep, num, den, 0, 0, 0, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, Kc, qpa, qbt;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb;

            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca = (*mpND_ca);
            Zca = (*mpND_Zca);
            cb = (*mpND_cb);
            Zcb = (*mpND_Zcb);
            xvin = (*xvmpND_in);

            //Dynamics of spool position (second order low pass filter)
            filter.update(xvin);
            xv = filter.value();

            //Determine flow coefficient
            Kc = Cq*f*pi*d*xv*sqrt(2.0/890.0);

            //Calculate flow
            qTurb__pa.setFlowCoefficient(Kc);
            qTurb__bt.setFlowCoefficient(Kc);
            qpa = qTurb__pa.getFlow(cp, ca, Zcp, Zca);
            qbt = qTurb__bt.getFlow(cb, ct, Zcb, Zct);

            if (xv >= 0.0)
            {
                qp = -qpa;
                qa = qpa;
                qb = -qbt;
                qt = qbt;
            }
            else
            {
                qp = 0;
                qa = 0;
                qb = 0;
                qt = 0;
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
        }
    };
}

#endif // HYDRAULIC42VALVE_HPP_INCLUDED
