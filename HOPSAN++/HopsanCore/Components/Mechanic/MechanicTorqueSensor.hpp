//!
//! @file   MechanicTorqueSensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-23
//!
//! @brief Contains a Mechanic Torque Sensor Component
//!
//$Id$

#ifndef MECHANICTORQUESENSOR_HPP_INCLUDED
#define MECHANICTORQUESENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorqueSensor : public ComponentSignal
    {
    private:
        double *mpND_t, *mpND_out;
        Port *mpP1, *mpOut;


    public:
        static Component *Creator()
        {
            return new MechanicTorqueSensor("TorqueSensor");
        }

        MechanicTorqueSensor(const std::string name) : ComponentSignal(name)
        {

            mpP1 = addReadPort("P1", "NodeMechanicRotational");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::TORQUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_t);
        }
    };
}

#endif // MECHANICTORQUESENSOR_HPP_INCLUDED
