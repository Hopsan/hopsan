//!
//! @file   Hydraulic33Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a hydraulic 3/3-valve of Q-type

#ifndef HYDRAULIC33VALVE_HPP_INCLUDED
#define HYDRAULIC33VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 3/3-valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic33Valve : public ComponentQ
    {
    private:
        double Cq;
        double d;
        double f;
        double xvmax;
        double overlap_pa;
        double overlap_at;
        double omegah;
        double deltah;
        double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;

        double *pa_ptr, *qa_ptr, *ca_ptr, *Zca_ptr, *pp_ptr, *qp_ptr, *cp_ptr, *Zcp_ptr, *pt_ptr, *qt_ptr, *ct_ptr, *Zct_ptr, *xvmpND_in;
        double pa, qa, ca, Zca, pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin;

        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        TurbulentFlowFunction mQturbat;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic33Valve("Hydraulic 3/3 Valve");
        }

        Hydraulic33Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic33Valve";
            Cq = 0.67;
            d = 0.01;
            f = 1.0;
            xvmax = 0.01;
            overlap_pa = 0.0;
            overlap_at = 0.0;
            omegah = 100.0;
            deltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", Cq);
            registerParameter("d", "Diameter", "[m]", d);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", f);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", xvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", overlap_pa);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", overlap_at);
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
            ct = (*ct_ptr);
            Zct = (*Zct_ptr);
            ca = (*ca_ptr);
            Zca = (*Zca_ptr);
            xvin = (*xvmpND_in);

            myFilter.update(xvin);
            xv = myFilter.value();

            xpanom = std::max(xv-overlap_pa,0.0);
            xatnom = std::max(-xv-overlap_at,0.0);

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/890.0);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            mQturbpa.setFlowCoefficient(Kcpa);
            mQturbat.setFlowCoefficient(Kcat);

            qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            qat = mQturbat.getFlow(ca, ct, Zca, Zct);

            if (xv >= 0.0)
            {
                qp = -qpa;
                qa = qpa;
                qt = 0;
            }
            else
            {
                qp = 0;
                qa = -qat;
                qt = qat;
            }

            //Write new values to nodes

            (*pp_ptr) = cp + qp*Zcp;
            (*qp_ptr) = qp;
            (*pa_ptr) = ca + qa*Zca;
            (*qa_ptr) = qa;
            (*pt_ptr) = ct + qt*Zct;
            (*qt_ptr) = qt;
        }
    };
}

#endif // HYDRAULIC33VALVE_HPP_INCLUDED
