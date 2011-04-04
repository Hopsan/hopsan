//$Id$

#ifndef HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED
#define HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#define M_PI       3.14159265358979323846

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVariableDisplacementPump : public ComponentQ
    {
    private:
        double n;             // rad/s
        double dp;
        double Kcp;
        double eps;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_eps;

        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicVariableDisplacementPump("VariableDisplacementPump");
        }

        HydraulicVariableDisplacementPump(const std::string name) : ComponentQ(name)
        {
            n = 125.0;
            dp = 0.00005;
            Kcp = 0.0;
            eps = 1.0;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("Speed", "Angular Velocity", "[rad/s]", n);
            registerParameter("Dp", "Displacement", "[m^3/rev]", dp);
            registerParameter("Kcp", "Leakage Coefficient", "[(m^3/s)/Pa]", Kcp);
            registerParameter("eps", "Displacement Setting", "[-]", eps);
        }


        void initialize()
        {
            mpND_eps = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, eps);

            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;
            bool cav = false;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            eps = (*mpND_eps);

            //Variable Displacement Pump equations

            q2 = ( dp*n*eps/(2.0*M_PI) + Kcp*(c1-c2) ) / ( (Zc1+Zc2)*Kcp+1 );
            q1 = -q2;
            p2 = c2 + Zc2*q2;
            p1 = c1 + Zc1*q1;

            /* Cavitation Check */

            if (p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if (p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if (cav)
            {
                q2 = ( dp*n*eps/(2.0*pi) + Kcp*(c1-c2) ) / ( (Zc1+Zc2)*Kcp+1 );
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED
