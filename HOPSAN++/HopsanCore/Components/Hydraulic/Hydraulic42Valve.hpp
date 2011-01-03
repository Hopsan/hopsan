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
        double xv, Kc, qpa, qbt;

        double *pp_ptr, *qp_ptr, *cp_ptr, *Zcp_ptr, *pt_ptr, *qt_ptr, *ct_ptr, *Zct_ptr, *pa_ptr, *qa_ptr, *ca_ptr, *Zca_ptr, *pb_ptr, *qb_ptr, *cb_ptr, *Zcb_ptr, *xvin_ptr;
        double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb;

        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturb_pa;
        TurbulentFlowFunction mQturb_bt;
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

            xvin_ptr = mpIn->getNodeDataPtr(NodeSignal::VALUE);

            //Initiate second order low pass filter
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            myFilter.initialize(mTimestep, num, den, 0, 0, 0, xvmax);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            cp = (*cp_ptr);
            Zcp = (*Zcp_ptr);
            ct = (*ct_ptr);
            Zct = (*Zct_ptr);
            ca = (*ca_ptr);
            Zca = (*Zca_ptr);
            cb = (*cb_ptr);
            Zcb = (*Zcb_ptr);
            xvin = (*xvin_ptr);

            //Dynamics of spool position (second order low pass filter)
            myFilter.update(xvin);
            xv = myFilter.value();

            //Determine flow coefficient
            Kc = Cq*f*pi*d*xv*sqrt(2.0/890.0);

            //Calculate flow
            mQturb_pa.setFlowCoefficient(Kc);
            mQturb_bt.setFlowCoefficient(Kc);
            qpa = mQturb_pa.getFlow(cp, ca, Zcp, Zca);
            qbt = mQturb_bt.getFlow(cb, ct, Zcb, Zct);

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

#endif // HYDRAULIC42VALVE_HPP_INCLUDED
