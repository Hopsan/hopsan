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
//! @file   HydraulicCsensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains a Hydraulic C Sensor Component (from Pressure sensor)
//!
//$Id: HydraulicCSensor.hpp 2013-03-28 08:15:57Z petkr14 $

#ifndef HYDRAULICCSENSOR_HPP_INCLUDED
#define HYDRAULICCSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicCsensor : public ComponentSignal
    {
    private:
        Port *mpP1, * mpOut;

        double *mpND_c, *mpND_out;

    public:
        static Component *Creator()
        {
            return new HydraulicCsensor();
        }

        void configure()
        {
            mpP1 = addReadPort("P1", "NodeHydraulic");
            mpOut = addWritePort("out", "NodeSignal",  Port::NotRequired);
        }


        void initialize()
        {
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
            mpOut->setSignalNodeUnitAndDescription("Pa","Pressure");
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_c);
        }
    };
}

#endif // HYDRAULICCSENSOR_HPP_INCLUDED
