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
     double *mpPp1_Qm, *mpOut;

public:
     static Component *Creator()
     {
        return new PneumaticQmsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic", "", Port::NotRequired);
        addOutputVariable("out", "Flow", "kg/s", &mpOut);
     }

    void initialize()
     {
        mpPp1_Qm=getSafeNodeDataPtr(mpPp1, NodePneumatic::MassFlow);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpOut) = (*mpPp1_Qm);
     }
};
#endif // PNEUMATICQMSENSOR_HPP_INCLUDED
