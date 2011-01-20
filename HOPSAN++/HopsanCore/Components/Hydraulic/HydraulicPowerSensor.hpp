//!
//! @file   HydraulicPowerSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic Power Sensor Component
//!
//$Id$

#ifndef HYDRAULICPOWERSENSOR_HPP_INCLUDED
#define HYDRAULICPOWERSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPowerSensor : public ComponentSignal
    {
    private:
        double *mpND_p, *mpND_q, *mpND_out;

        Port *mpP1, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicPowerSensor("PowerSensor");
        }

        HydraulicPowerSensor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "HydraulicPowerSensor";

            mpP1 = addReadPort("P1", "NodeHydraulic");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_p) * (*mpND_q);
        }
    };
}

#endif // HYDRAULICPOWERSENSOR_HPP_INCLUDED
