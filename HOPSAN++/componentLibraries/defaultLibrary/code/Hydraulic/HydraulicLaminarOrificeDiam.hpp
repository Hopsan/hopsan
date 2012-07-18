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

//$Id$

#ifndef HYDRAULICLAMINARORIFICEDIAM_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICEDIAM_HPP_INCLUDED

#include <iostream>
#include <cmath>
#include "ComponentEssentials.h"

namespace hopsan {
using namespace std;

    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLaminarOrificeDiam : public ComponentQ
    {
    private:
        double Kc, dh, rho;
        bool cav;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_dh;

        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrificeDiam();
        }

        HydraulicLaminarOrificeDiam() : ComponentQ()
        {
            dh = .001;
            rho = 890;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addReadPort("d_h", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("rho", "Oil density", "[kg/m^3]", rho);
            registerParameter("d_h", "Hydraulic diameter", "[m^2]", dh);
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

            mpND_dh = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, dh);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            p1 = (*mpND_p1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            p2 = (*mpND_p2);
            Zc2 = (*mpND_Zc2);
            dh = (*mpND_dh);

            Kc = 2.0/rho*.67*dh*dh/sqrt(2.0/rho*abs(p1-p2));

            //Orifice equations
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICLAMINARORIFICEDIAM_HPP_INCLUDED
