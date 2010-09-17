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
        double mSpeed;             // rad/s
        double mDp;
        double mKcp;
        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicVariableDisplacementPump("VariableDisplacementPump");
        }

        HydraulicVariableDisplacementPump(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicVariableDisplacementPump";
            mSpeed = 125.0;
            mDp = 0.00005;
            mKcp = 0.0;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Speed", "Angular Velocity", "rad/s", mSpeed);
            registerParameter("Dp", "Displacement", "m^3/rev", mDp);
            registerParameter("Kcp", "Leakage Coefficient", "(m^3/s)/Pa", mKcp);
        }


        void initialize()
        {
            //Nothing to initilize
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
            double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);
            double eps = mpIn->readNode(NodeSignal::VALUE);

            //Variable Displacement Pump equations

            double q2 = ( mDp*mSpeed*eps/(2.0*M_PI) + mKcp*(c1-c2) ) / ( (Zc1+Zc2)*mKcp+1 );
            double q1 = -q2;
            double p2 = c2 + Zc2*q2;
            double p1 = c1 + Zc1*q1;

            /* Cavitation Check */

            bool cav = false;

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
                q2 = ( mDp*mSpeed*eps/(2.0*pi) + mKcp*(c1-c2) ) / ( (Zc1+Zc2)*mKcp+1 );
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
            mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
            mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
            mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);
        }
    };
}

#endif // HYDRAULICVARIABLEDISPLACEMENTPUMP_HPP_INCLUDED
