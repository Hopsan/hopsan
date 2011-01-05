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
        double *f_ptr, *out_ptr;
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
            if(mpP1->isConnected())
            {
                f_ptr = mpP1->getNodeDataPtr(NodeMechanic::FORCE);
            }
            else
            {
                f_ptr = new double(0);
            }
            if(mpOut->isConnected())
            {
                out_ptr = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                out_ptr = new double();
            }
        }


        void simulateOneTimestep()
        {
            (*out_ptr) = (*f_ptr);
        }
    };
}

#endif // MECHANICFORCESENSOR_HPP_INCLUDED
