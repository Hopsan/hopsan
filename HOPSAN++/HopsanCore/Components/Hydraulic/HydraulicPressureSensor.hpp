//!
//! @file   HydraulicPressureSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic Pressure Sensor Component
//!
//$Id$

#ifndef HYDRAULICPRESSURESENSOR_HPP_INCLUDED
#define HYDRAULICPRESSURESENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSensor : public ComponentSignal
    {
    private:
        Port *mpP1, * mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSensor("PressureSensor");
        }

        HydraulicPressureSensor(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "HydraulicPressureSensor";

            mpP1 = addReadPort("P1", "NodeHydraulic");
            mpOut = addWritePort("out", "NodeSignal",  Port::NOTREQUIRED);
        }


        void initialize()
        {
            //Nothing to initilize
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double p = mpP1->readNode(NodeHydraulic::PRESSURE);

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, p);
        }
    };
}

#endif // HYDRAULICPRESSURESENSOR_HPP_INCLUDED
