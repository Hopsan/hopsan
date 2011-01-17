//!
//! @file   HydraulicLosslessTConnector.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-04
//!
//! @brief Contains a Hydraulic Lossless Connector with 3 ports
//!

#ifndef HYDRAULICLOSSLESSTCONNECTOR_HPP_INCLUDED
#define HYDRAULICLOSSLESSTCONNECTOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic lossless T-connector component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLosslessTConnector : public ComponentQ
    {
    private:
        double Kc;
        double p;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *p3_ptr, *q3_ptr, *c3_ptr, *Zc3_ptr;
        double q1, q2, q3, c1, Zc1, c2, Zc2, c3, Zc3;

        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicLosslessTConnector("LosslessTConnector");
        }

        HydraulicLosslessTConnector(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicLosslessTConnector";
            Kc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic");
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

            p3_ptr = mpP3->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q3_ptr = mpP3->getNodeDataPtr(NodeHydraulic::FLOW);
            c3_ptr = mpP3->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc3_ptr = mpP3->getNodeDataPtr(NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            c3 = (*c3_ptr);
            Zc3 = (*Zc3_ptr);

            //T-Connector equations
            p = (c1/Zc1 + c2/Zc2 + c3/Zc3) / ( 1/Zc1 + 1/Zc2 + 1/Zc3);
            q1 = (p-c1)/Zc1;
            q2 = (p-c2)/Zc2;
            q3 = (p-c3)/Zc3;

            //Cavitation check
            if(p < 0.0)
            {
                p = 0.0;
            }

            //Write new variables to nodes
            (*mpND_p1) = p;
            (*mpND_q1) = q1;
            (*mpND_p2) = p;
            (*mpND_q2) = q2;
            (*mpND_p2) = p;
            (*q3_ptr) = q3;
        }
    };
}

#endif // HYDRAULICLOSSLESSTCONNECTOR_HPP_INCLUDED

