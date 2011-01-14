//!
//! @file   MechanicSpeedSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Speed Sensor Component
//!
//$Id$

#ifndef MECHANICFORCESENSOR_HPP_INCLUDED
#define MECHANICFORCESENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicForceSensor : public ComponentSignal
    {
    private:
        double *mpND_f, *mpND_out;
        Port *mpP1, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicForceSensor("ForceSensor");
        }

        MechanicForceSensor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "MechanicForceSensor";

            mpP1 = addReadPort("P1", "NodeMechanic", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_f = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_f);
        }
    };
}

#endif // MECHANICFORCESENSOR_HPP_INCLUDED
