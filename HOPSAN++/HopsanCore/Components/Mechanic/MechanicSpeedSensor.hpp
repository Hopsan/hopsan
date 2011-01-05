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
        double *v_ptr, *out_ptr;
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
            if(mpP1->isConnected()) { v_ptr = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY); }
            else { v_ptr = new double(0); }

            if(mpOut->isConnected()) { out_ptr = mpOut->getNodeDataPtr(NodeSignal::VALUE); }
            else { out_ptr = new double(); }
        }


        void simulateOneTimestep()
        {
            (*out_ptr) = (*v_ptr);
        }
    };
}

#endif // MECHANICSPEEDSENSOR_HPP_INCLUDED
