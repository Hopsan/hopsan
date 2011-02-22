//!
//! @file   MechanicAngleSensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-12
//!
//! @brief Contains a Mechanic Angle Sensor Component
//!
//$Id$

#ifndef MECHANICANGLESENSOR_HPP_INCLUDED
#define MECHANICANGLESENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngleSensor : public ComponentSignal
    {
    private:
        double *mpND_phi, *mpND_out;
        Port *mpP1, *mpOut;


    public:
        static Component *Creator()
        {
            return new MechanicAngleSensor("AngleSensor");
        }

        MechanicAngleSensor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "MechanicAngleSensor";

            mpP1 = addReadPort("P1", "NodeMechanicRotational");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            mpND_phi = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::ANGLE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_phi);
        }
    };
}

#endif // MECHANICANGLESENSOR_HPP_INCLUDED
