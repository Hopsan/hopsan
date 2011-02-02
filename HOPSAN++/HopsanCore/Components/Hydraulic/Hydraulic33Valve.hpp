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

        double *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_xvin;

        SecondOrderFilter filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_at;
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
            deltah = 1.0;

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

            mpND_xvin = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);

            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            filter.initialize(mTimestep, num, den, 0, 0, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;
            double pp, pt, pa, qa, ca, Zca, qp, cp, Zcp, qt, ct, Zct, xvin;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca = (*mpND_ca);
            Zca = (*mpND_Zca);
            xvin = (*mpND_xvin);

            filter.update(xvin);
            xv = filter.value();

            xpanom = std::max(xv-overlap_pa,0.0);
            xatnom = std::max(-xv-overlap_at,0.0);

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/890.0);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_at.setFlowCoefficient(Kcat);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

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

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
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
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

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

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
            }

            //Write new values to nodes
            (*mpND_pp) = pp;
            (*mpND_qp) = qp;
            (*mpND_pa) = pa;
            (*mpND_qa) = qa;
            (*mpND_pt) = pt;
            (*mpND_qt) = qt;
        }
    };
}

#endif // HYDRAULIC33VALVE_HPP_INCLUDED
