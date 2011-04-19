//!
//! @file   HydraulicUndefinedConnectionQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-03-09
//!
//! @brief Contains a Hydraulic Undefined Connection Component of Q-type
//!
//$Id: HydraulicUndefinedConnectionQ.hpp 2510 2011-01-26 13:05:20Z robbr48 $

#ifndef HYDRAULICUNDEFINEDCONNECTIONQ_HPP_INCLUDED
#define HYDRAULICUNDEFINEDCONNECTIONQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicUndefinedConnectionQ : public ComponentQ
    {
    private:
        Port *mpP1;
        double P, Q;
        double *mpND_p, *mpND_q;

    public:
        static Component *Creator()
        {
            return new HydraulicUndefinedConnectionQ("UndefinedConnectionQ");
        }

        HydraulicUndefinedConnectionQ(const std::string name) : ComponentQ(name)
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("P",  "Pressure",     "[-]",  P);
            registerParameter("Q",  "Flow",         "[-]",  Q);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);

            (*mpND_p) = P;
            (*mpND_q) = Q;
        }


        void simulateOneTimestep()
        {
            (*mpND_p) = P;
            (*mpND_q) = Q;
        }
    };
}

#endif // HYDRAULICUNDEFINEDCONNECTIONQ_HPP_INCLUDED
