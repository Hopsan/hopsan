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
//! @file   Hydraulic22DirectionalValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-24
//!
//! @brief Contains a hydraulic on/off valve of Q-type

#ifndef HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED
#define HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED



#include <iostream>
#include <sstream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic on/off 2/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22DirectionalValve : public ComponentQ
    {
    private:
        double *mpCq, *mpD, *mpF, *mpXvmax, *mpRho;
        double omegah;
        double deltah;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;
        double *mpND_xvin, *mpND_xvout;

        SecondOrderTransferFunction filter;
        TurbulentFlowFunction qTurb;
        Port *mpP1, *mpP2, *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new Hydraulic22DirectionalValve();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("xv", "NodeSignal", Port::NotRequired);

            addInputVariable("C_q", "Flow Coefficient", "[-]", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "[kg/m^3]", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "[m]", 0.01, &mpD);
            addInputVariable("f", "Spool Fraction of the Diameter", "[-]", 1.0, &mpF);
            addInputVariable("x_vmax", "Maximum Spool Displacement", "[m]", 0.01, &mpXvmax);

            registerParameter("omega_h", "Resonance Frequency", "[rad/s]", omegah);
            registerParameter("delta_h", "Damping Factor", "[-]", deltah);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            mpND_xvin = getSafeNodeDataPtr(mpIn, NodeSignal::Value);
            mpND_xvout = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            filter.initialize(mTimestep, num, den, 0, 0, 0, (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xnom, Kc, q, Cq, rho, d, f, xvmax;
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, xvin;
            bool cav = false;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            xvin = (*mpND_xvin);

            Cq = (*mpCq);
            rho = (*mpRho);
            d = (*mpD);
            f = (*mpF);
            xvmax = (*mpXvmax);

            if(doubleToBool(xvin))
            {
                filter.update(xvmax);
            }
            else
            {
                filter.update(0);
            }

            xv = filter.value();

            xnom = std::max(xv,0.0);

            Kc = Cq*f*pi*d*xnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb.setFlowCoefficient(Kc);

            q = qTurb.getFlow(c1, c2, Zc1, Zc2);

            q1 = -q;
            q2 = q;

            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0;
                cav = true;
            }

            if(cav)
            {
                q = qTurb.getFlow(c1, c2, Zc1, Zc2);

                q1 = -q;
                q2 = q;

                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
            }

            //Write new values to nodes

            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_xvout) = Kc;
        }
    };
}

#endif // HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED

