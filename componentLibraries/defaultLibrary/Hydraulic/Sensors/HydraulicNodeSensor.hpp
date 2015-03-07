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
//! @file   HydraulicNodeSensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-03-07
//!
//! @brief Contains a Hydraulic Node Sensor Component
//!
//$Id: HydraulicPressureSensor.hpp 7574 2015-01-27 12:46:45Z petno25 $

#ifndef HYDRAULICNODESENSOR_HPP_INCLUDED
#define HYDRAULICNODESENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicNodeSensor : public ComponentSignal
    {
    private:
        Port *mpP1;
        double *mpOut_p, *mpOut_q, *mpOut_c, *mpOut_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicNodeSensor();
        }

        void configure()
        {
            mpP1 = addReadPort("P1", "NodeHydraulic", "", Port::NotRequired);
            addOutputVariable("p", "Pressure", "Pa", &mpOut_p);
            addOutputVariable("q", "Flow", "m^3/s", &mpOut_q);
            addOutputVariable("c", "WaveVariable", "",  &mpOut_c);
            addOutputVariable("Zc", "Charateristc Impedance", "", &mpOut_Zc);
        }


        void initialize()
        {
            simulateOneTimestep(); //Set initial output node value
        }


        void simulateOneTimestep()
        {
            *mpOut_p = mpP1->readNode(NodeHydraulic::Pressure);
            *mpOut_q = mpP1->readNode(NodeHydraulic::Flow);
            *mpOut_c = mpP1->readNode(NodeHydraulic::WaveVariable);
            *mpOut_Zc = mpP1->readNode(NodeHydraulic::CharImpedance);
        }
    };
}

#endif
