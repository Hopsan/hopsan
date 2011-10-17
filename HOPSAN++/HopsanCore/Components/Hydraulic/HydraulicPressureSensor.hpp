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
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSensor : public ComponentSignal
    {
    private:
        Port *mpP1, * mpOut;

        double *mpND_p, *mpND_out;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSensor("PressureSensor");
        }

        HydraulicPressureSensor(const std::string name) : ComponentSignal(name)
        {

            mpP1 = addReadPort("P1", "NodeHydraulic");
            mpOut = addWritePort("out", "NodeSignal",  Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_p);
        }
    };
}

#endif // HYDRAULICPRESSURESENSOR_HPP_INCLUDED
