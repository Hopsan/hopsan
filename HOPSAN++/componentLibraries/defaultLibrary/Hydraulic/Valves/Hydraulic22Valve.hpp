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
//! @file   Hydraulic22Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 2/2-valve of Q-type

#ifndef HYDRAULIC22VALVE_HPP_INCLUDED
#define HYDRAULIC22VALVE_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 2/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22Valve : public ComponentQ
    {
    private:
        double *mpCq, *mpD, *mpF, *mpXvmax, *mpRho, *mpXv;

        double omegah;
        double deltah;
        SecondOrderTransferFunction filter;
        TurbulentFlowFunction qTurb_pa;
        double xpanom, Kcpa, qpa;

        double *mpND_cp, *mpND_Zcp, *mpND_ca, *mpND_Zca, *mpND_pp, *mpND_qp, *mpND_pa, *mpND_qa;
        double *mpXvIn;

        Port *mpPP, *mpPA;

    public:
        static Component *Creator()
        {
            return new Hydraulic22Valve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");

            addInputVariable("in", "Desired spool position", "", 0.0, &mpXvIn);
            addOutputVariable("xv", "Spool position", "", 0.0, &mpXv);
            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "kg/m^3", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f", "Spool Fraction of the Diameter", "-", 1.0, &mpF);
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

            mpND_pa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpND_qa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpND_ca = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpND_Zca = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            double xvmax = (*mpXvmax);

            //Initiate second order low pass filter
            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            filter.initialize(mTimestep, num, den, 0, 0, 0, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cp, Zcp, ca, Zca, xvin, pp, qp, pa, qa, Cq, rho, d, f, xvmax, xv;
            bool cav = false;

            //Read variables from nodes
            cp = (*mpND_cp);
            Zcp = (*mpND_Zcp);
            ca = (*mpND_ca);
            Zca = (*mpND_Zca);
            xvin = (*mpXvIn);

            Cq = (*mpCq);
            rho = (*mpRho);
            d = (*mpD);
            f = (*mpF);
            xvmax = (*mpXvmax);

            //Dynamics of spool position (second order low pass filter)
            limitValue(xvin, 0, xvmax);
            filter.update(xvin);
            xv = filter.value();

            //Determine flow coefficient
            xpanom = xv;
            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/rho);

            //Calculate flow
            qTurb_pa.setFlowCoefficient(Kcpa);
            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);

            qp = -qpa;
            qa = qpa;

            pp = cp + qp*Zcp;
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

            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);

                if (xv >= 0.0)
                {
                    qp = -qpa;
                    qa = qpa;
                }
                else
                {
                    qp = 0;
                    qa = 0;
                }

                pp = cp + qp*Zcp;
                pa = ca + qa*Zca;
            }

            //Calculate pressures from flow and impedance
            (*mpND_pp) = pp;
            (*mpND_qp) = qp;
            (*mpND_pa) = pa;
            (*mpND_qa) = qa;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC22VALVE_HPP_INCLUDED
