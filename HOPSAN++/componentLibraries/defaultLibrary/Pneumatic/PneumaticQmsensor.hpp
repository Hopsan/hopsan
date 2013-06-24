#ifndef PNEUMATICQMSENSOR_HPP_INCLUDED
#define PNEUMATICQMSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticQmsensor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 13 Mar 2013 16:55:00
//! @brief Pneumatic massflow sensor
//! @ingroup PneumaticComponents
//!
//$Id$

using namespace hopsan;

class PneumaticQmsensor : public ComponentSignal
{
private:
     Port *mpPp1;
     double *mpND_pp1, *mpND_qmsensor;

public:
     static Component *Creator()
     {
        return new PneumaticQmsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic");
        addOutputVariable("out", "Flow", "kg/s", &mpND_qmsensor);
     }

    void initialize()
     {
        mpND_pp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::Pressure);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpND_qmsensor) = (*mpND_pp1);
     }
};
#endif // PNEUMATICQMSENSOR_HPP_INCLUDED
