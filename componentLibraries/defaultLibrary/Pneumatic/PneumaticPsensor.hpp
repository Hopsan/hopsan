#ifndef PNEUMATICPSENSOR_HPP_INCLUDED
#define PNEUMATICPSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticPsensor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 13 Mar 2013 16:11:02
//! @brief Pneumatic pressure and temperature source
//! @ingroup PneumaticComponents
//!
//$Id$

using namespace hopsan;

class PneumaticPsensor : public ComponentSignal
{
private:
     Port *mpPp1;
     double *mpPp1_P, *mpOut;

public:
     static Component *Creator()
     {
        return new PneumaticPsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic", "", Port::NotRequired);
        addOutputVariable("out", "Pressure", "Pa", &mpOut);
     }

    void initialize()
     {
        mpPp1_P=getSafeNodeDataPtr(mpPp1, NodePneumatic::Pressure);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpOut) = (*mpPp1_P);
     }
};
#endif // PNEUMATICPSENSOR_HPP_INCLUDED
