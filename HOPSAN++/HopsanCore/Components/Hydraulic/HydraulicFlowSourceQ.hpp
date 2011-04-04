//!
//! @file   HydraulicFlowSourceQ.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Flow Source Component of Q-type
//!
//$Id$

#ifndef HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
#define HYDRAULICFLOWSOURCEQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicFlowSourceQ : public ComponentQ
    {
    private:
        double mFlow;
        double *mpND_in, *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicFlowSourceQ("FlowSourceQ");
        }

        HydraulicFlowSourceQ(const std::string name) : ComponentQ(name)
        {
            mFlow = 1.0e-3;

            mpIn = addReadPort("in", "NodeSignal",  Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("Flow", "Flow", "[m^3/s]", mFlow);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, mFlow);
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double in, p, q, c, Zc;

            //Read variables from nodes
            in = (*mpND_in);
            c = (*mpND_c);
            Zc = (*mpND_Zc);

            //Flow source equations
            q = in;
            p = c + q*Zc;

            (*mpND_p) = p;
            (*mpND_q) = q;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
