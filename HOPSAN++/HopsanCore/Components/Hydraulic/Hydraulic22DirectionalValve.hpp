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

#define pi 3.14159

#include <iostream>
#include <sstream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic on/off 2/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22DirectionalValve : public ComponentQ
    {
    private:
        double Cq;
        double d;
        double f;
        double xvmax;
        double rho;
        double omegah;
        double deltah;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;
        double *mpND_xvin, *mpND_xvout;

        SecondOrderFilter filter;
        TurbulentFlowFunction qTurb;
        Port *mpP1, *mpP2, *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new Hydraulic22DirectionalValve("Hydraulic directional 2/2 Valve");
        }

        Hydraulic22DirectionalValve(const std::string name) : ComponentQ(name)
        {
            Cq = 0.67;
            d = 0.01;
            f = 1.0;
            xvmax = 0.01;
            rho = 890;
            omegah = 100.0;
            deltah = 1.0;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
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
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            mpND_xvin = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_xvout = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(omegah*omegah), 2.0*deltah/omegah, 1.0};
            filter.initialize(mTimestep, num, den, 0, 0, -xvmax, xvmax);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xnom, Kc, q;
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, xvin;
            bool cav = false;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
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

                q1 = q;
                q2 = -q;

                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
            }


            //Write new values to nodes

            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_xvout) = xv;
        }
    };
}

#endif // HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED

