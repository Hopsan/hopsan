//!
//! @file   MechanicSpeedSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Speed Sensor Component
//!
//$Id$

#ifndef MECHANICSPEEDSENSOR_HPP_INCLUDED
#define MECHANICSPEEDSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicSpeedSensor : public ComponentSignal
    {
    private:
        double *mpND_v, *mpND_out;
        Port *mpP1, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicSpeedSensor("SpeedSensor");
        }

        MechanicSpeedSensor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "MechanicSpeedSensor";

            mpP1 = addReadPort("P1", "NodeMechanic", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_v = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_v);
        }
    };
}

#endif // MECHANICSPEEDSENSOR_HPP_INCLUDED
