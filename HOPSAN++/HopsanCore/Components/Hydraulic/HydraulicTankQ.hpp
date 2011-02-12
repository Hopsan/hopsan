//!
//! @file   HydraulicTankQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-12
//!
//! @brief Contains a Hydraulic Tank Component of Q-type
//!
//$Id: HydraulicTankQ.hpp 2510 2011-01-26 13:05:20Z robbr48 $

#ifndef HYDRAULICTANKQ_HPP_INCLUDED
#define HYDRAULICTANKQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTankQ : public ComponentQ
    {
    private:
        double p;

        double *mpND_in, *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicTankQ("TankQ");
        }

        HydraulicTankQ(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicTankQ";
            p = 1e5;

            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("p", "Default Pressure", "Pa", p);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q, c, Zc;

            //Get variable values from nodes
            c = (*mpND_c);
            Zc = (*mpND_Zc);

            //Equations
            q = (p - c)/Zc;

            //Write variables to nodes
            (*mpND_p) = p;
            (*mpND_q) = q;
        }
    };
}

#endif // HYDRAULICTANKQ_HPP_INCLUDED
