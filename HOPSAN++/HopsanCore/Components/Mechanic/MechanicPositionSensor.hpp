//!
//! @file   MechanicPositionSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Position Sensor Component
//!
//$Id$

#ifndef MECHANICPOSITIONSENSOR_HPP_INCLUDED
#define MECHANICPOSITIONSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicPositionSensor : public ComponentSignal
    {
    private:
        double *x_ptr, *out_ptr;
        Port *mpP1, *mpOut;


    public:
        static Component *Creator()
        {
            return new MechanicPositionSensor("PositionSensor");
        }

        MechanicPositionSensor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "MechanicPositionSensor";

            mpP1 = addReadPort("P1", "NodeMechanic", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            if(mpP1->isConnected()) { x_ptr = mpP1->getNodeDataPtr(NodeMechanic::POSITION); }
            else { x_ptr = new double(0); }

            if(mpOut->isConnected()) { out_ptr = mpOut->getNodeDataPtr(NodeSignal::VALUE); }
            else { out_ptr = new double(); }
        }


        void simulateOneTimestep()
        {
            (*out_ptr) = (*x_ptr);
        }
    };
}

#endif // MECHANICPOSITIONSENSOR_HPP_INCLUDED
