/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicFlowSensor : public ComponentSignal
    {
    private:
        double *mpND_q, *mpND_out;

        Port *mpP1, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicFlowSensor();
        }

        void configure()
        {

            mpP1 = addReadPort("P1", "NodeHydraulic");
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

        }


        void initialize()
        {
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
            mpOut->setSignalNodeUnitAndDescription("m^3/s", "Flow");
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_q);
        }
    };
}

#endif // HYDRAULICFLOWSENSOR_HPP_INCLUDED
