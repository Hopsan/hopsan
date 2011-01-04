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

        double *p1_ptr, *q1_ptr, *c1_ptr, *Zc1_ptr, *p2_ptr, *q2_ptr, *c2_ptr, *Zc2_ptr;
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
            p1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            c1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            p2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::FLOW);
            c2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*c1_ptr);
            Zc1 = (*Zc1_ptr);
            c2 = (*c2_ptr);
            Zc2 = (*Zc2_ptr);

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
            (*p1_ptr) = p1;
            (*q1_ptr) = -q;
            (*p2_ptr) = p2;
            (*q2_ptr) = q;
        }
    };
}

#endif // HYDRAULICLOSSLESSCONNECTOR_HPP_INCLUDED
