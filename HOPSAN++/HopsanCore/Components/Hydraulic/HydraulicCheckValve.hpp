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

        double *p1_ptr, *q1_ptr, *c1_ptr, *Zc1_ptr, *p2_ptr, *q2_ptr, *c2_ptr, *Zc2_ptr;
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
            p1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            c1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            p2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::FLOW);
            c2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);

            mQturb.setFlowCoefficient(mKs);
        }


        void simulateOneTimestep()
        {
              //Get variable values from nodes
            c1 = (*c1_ptr);
            Zc1 = (*Zc1_ptr);
            c2 = (*c2_ptr);
            Zc2 = (*Zc2_ptr);

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
            (*p1_ptr) = p1;
            (*q1_ptr) = q1;
            (*p2_ptr) = p2;
            (*q2_ptr) = q2;
        }
    };
}

#endif // HYDRAULICCHECKVALVE_HPP_INCLUDED
