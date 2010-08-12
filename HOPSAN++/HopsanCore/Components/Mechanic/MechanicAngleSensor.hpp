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
            //Nothing to initilize
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double phi = mpP1->readNode(NodeMechanicRotational::ANGLE);

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, phi);
        }
    };
}

#endif // MECHANICANGLESENSOR_HPP_INCLUDED
