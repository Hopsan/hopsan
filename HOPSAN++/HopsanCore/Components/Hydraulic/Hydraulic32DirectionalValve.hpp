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
//! @file   Hydraulic32DirectionalValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a hydraulic directional3/2-valve of Q-type

#ifndef HYDRAULIC32DIRECTIONALVALVE_HPP_INCLUDED
#define HYDRAULIC32DIRECTIONALVALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic directional 3/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic32DirectionalValve : public ComponentQ
    {
    private:
        double Cq;
        double d;
        double f;
        double xvmax;
        double rho;
        double omegah;
        double deltah;

        double *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct;
        double *mpND_xvin, *mpND_xvout;

        SecondOrderFilter filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_at;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new Hydraulic32DirectionalValve("Hydraulic directional 3/2 Valve");
        }

        Hydraulic32DirectionalValve(const std::string name) : ComponentQ(name)
        {
            Cq = 0.67;
            d = 0.01;
            f = 1.0;
            xvmax = 0.01;
            rho = 890;
            omegah = 100.0;
            deltah = 1.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("xv", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("C_q", "Flow Coefficient", "[-]", Cq);
            registerParameter("rho", "Oil Density", "[kg/m^3]", rho);
            registerParameter("d", "Diameter", "[m]", d);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", f);
            registerParameter("x_v,max", "Maximum Spool Displacement", "[m]", xvmax);
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

            mpND_xvin = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_xvout = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            filter.initialize(mTimestep, num, den, 0, 0, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;
            double pa, qa, ca, Zca, pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ct = (*mpND_ct);
            Zct = (*mpND_Zct);
            ca = (*mpND_ca);
            Zca = (*mpND_Zca);
            xvin = (*mpND_xvin);

            if(doubleToBool(xvin))
            {
                filter.update(xvmax);
            }
            else
            {
                filter.update(-xvmax);
            }

            xv = filter.value();

            xpanom = std::max(xv,0.0);
            xatnom = std::max(-xv,0.0);

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/rho);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/rho);

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
            pa = ca + qa*Zca;
            pt = ct + qt*Zct;

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
                pa = ca + qa*Zca;
                pt = ct + qt*Zct;
            }


            //Write new values to nodes

            (*mpND_pp) = pp;
            (*mpND_qp) = qp;
            (*mpND_pa) = pa;
            (*mpND_qa) = qa;
            (*mpND_pt) = pt;
            (*mpND_qt) = qt;
            (*mpND_xvout) = xv;
        }
    };
}

#endif // HYDRAULIC32DIRECTIONALVALVE_HPP_INCLUDED

