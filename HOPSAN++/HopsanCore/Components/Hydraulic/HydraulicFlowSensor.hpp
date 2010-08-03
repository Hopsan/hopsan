//!
//! @file   HydraulicFlowSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic Flow Sensor Component
//!
//$Id$

#ifndef HYDRAULICFLOWSENSOR_HPP_INCLUDED
#define HYDRAULICFLOWSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicFlowSensor : public ComponentSignal
{
private:
    Port *mpP1, *mpOut;

public:
    static Component *Creator()
    {
        return new HydraulicFlowSensor("FlowSensor");
    }

    HydraulicFlowSensor(const string name) : ComponentSignal(name)
    {
        mTypeName = "HydraulicFlowSensor";

        mpP1 = addReadPort("P1", "NodeHydraulic");
        mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q = mpP1->readNode(NodeHydraulic::MASSFLOW);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, q);
    }
};

#endif // HYDRAULICFLOWSENSOR_HPP_INCLUDED
