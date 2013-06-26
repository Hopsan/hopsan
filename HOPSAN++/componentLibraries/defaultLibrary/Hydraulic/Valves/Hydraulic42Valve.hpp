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
//! @file   Hydraulic42Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 4/2-valve of Q-type

#ifndef HYDRAULIC42VALVE_HPP_INCLUDED
#define HYDRAULIC42VALVE_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic42Valve : public ComponentQ
    {
    private:
        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_bt, *mpXvmax, *mpRho;
        double omegah, deltah;

        double *mpND_pp, *mpND_qp, *mpND_cp, *mpND_Zcp, *mpND_pt, *mpND_qt, *mpND_ct, *mpND_Zct, *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca, *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb;

        SecondOrderTransferFunction filter;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_bt;
        Port *mpPP, *mpPT, *mpPA, *mpPB;

    public:
        static Component *Creator()
        {
            return new Hydraulic42Valve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);
            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);
            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "kg/m^3", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "-", 1.0, &mpF_bt);
            addInputVariable("x_vmax", "Maximum Spool Displacement", "m", 0.01, &mpXvmax);

            addConstant("omega_h", "Resonance Frequency", "rad/s", 100.0, omegah);
            addConstant("delta_h", "Damping Factor", "-", 1.0, deltah);
        }


        void initialize()
        {
            mpND_pp = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpND_qp = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpND_cp = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpND_Zcp = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);

            mpND_pt = getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
            mpND_qt = getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
            mpND_ct = getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
            mpND_Zct = getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);

            mpND_pa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpND_qa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpND_ca = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpND_Zca = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            mpND_pb = getSafeNodeDataPtr(mpPB, NodeHydraulic::Pressure);
            mpND_qb = getSafeNodeDataPtr(mpPB, NodeHydraulic::Flow);
            mpND_cb = getSafeNodeDataPtr(mpPB, NodeHydraulic::WaveVariable);
            mpND_Zcb = getSafeNodeDataPtr(mpPB, NodeHydraulic::CharImpedance);

            //Initiate second order low pass filter
            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            filter.initialize(mTimestep, num, den, 0, 0, 0, (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, Kcpa, Kcbt, qpa, qbt;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, Cq, rho, xvmax, d, f_pa, f_bt;
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

            xvin = (*mpXvIn);
            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_bt = (*mpF_bt);

            //Dynamics of spool position (second order low pass filter)
            limitValue(xvin, 0, xvmax);
            filter.update(xvin);
            xv = filter.value();

            //Determine flow coefficient
            Kcpa = Cq*f_pa*pi*d*xv*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xv*sqrt(2.0/rho);

            //Calculate flow
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_bt.setFlowCoefficient(Kcbt);
            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
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
                qp = 0;
                qa = 0;
                qb = 0;
                qt = 0;
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
                    qp = 0;
                    qa = 0;
                    qb = 0;
                    qt = 0;
                }

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
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
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC42VALVE_HPP_INCLUDED
