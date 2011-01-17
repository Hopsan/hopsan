//!
//! @file   HydraulicCheckValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-07
//!
//! @brief Contains a Hydraulic Checkvalve component
//!
//$Id$

#ifndef HYDRAULICCHECKVALVE_HPP_INCLUDED
#define HYDRAULICCHECKVALVE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicCheckValve : public ComponentQ
    {
    private:
        double mKs;
        bool cav;
        TurbulentFlowFunction mQturb;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;
        double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicCheckValve("CheckValve");
        }

        HydraulicCheckValve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicCheckValve";
            mKs = 0.000000025;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            registerParameter("Ks", "Restrictor Coefficient", "-", mKs);
        }


        void initialize()
        {
            mpND_p1 = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            mpND_q1 = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            mpND_c1 = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            mpND_p2 = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            mpND_q2 = mpP2->getNodeDataPtr(NodeHydraulic::FLOW);
            mpND_c2 = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);

            mQturb.setFlowCoefficient(mKs);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);

            //Checkvalve equations
            if (c1 > c2) { q2 = mQturb.getFlow(c1, c2, Zc1, Zc2); }
            else { q2 = 0.0; }

            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;

            //Cavitation check
            cav = false;
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
                if (c1 > c2) { q2 = mQturb.getFlow(c1, c2, Zc1, Zc2); }
                else { q2 = 0.0; }
            }
            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;
            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICCHECKVALVE_HPP_INCLUDED
