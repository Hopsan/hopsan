//!
//! @file   HydraulicLosslessConnector.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-04
//!
//! @brief Contains a Hydraulic Lossless Connector ("lossless orifice")
//!

#ifndef HYDRAULICLOSSLESSCONNECTOR_HPP_INCLUDED
#define HYDRAULICLOSSLESSCONNECTOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic lossless connector component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLosslessConnector : public ComponentQ
    {
    private:
        double Kc;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;
        double q, p1, p2, c1, Zc1, c2, Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicLosslessConnector("LosslessConnector");
        }

        HydraulicLosslessConnector(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicLosslessConnector";
            Kc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
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
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);

            //Connector equations
            q = (c1-c2)/(Zc1+Zc2);
            p1 = c1 - q*Zc1;
            p2 = c2 + q*Zc2;

            //Cavitation check
            if(p1 < 0.0)
            {
                p1 = 0.0;
            }
            if(p2 < 0.0)
            {
                p2 = 0.0;
            }

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = -q;
            (*mpND_p2) = p2;
            (*mpND_q2) = q;
        }
    };
}

#endif // HYDRAULICLOSSLESSCONNECTOR_HPP_INCLUDED
