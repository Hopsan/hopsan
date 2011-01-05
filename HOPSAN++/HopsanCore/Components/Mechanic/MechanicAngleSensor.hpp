//!
//! @file   MechanicAngleSensor.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
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
        double *phi_ptr, *out_ptr;
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
            phi_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::ANGLE);
            out_ptr = mpOut->getNodeDataPtr(NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*out_ptr) = (*phi_ptr);
        }
    };
}

#endif // MECHANICANGLESENSOR_HPP_INCLUDED
