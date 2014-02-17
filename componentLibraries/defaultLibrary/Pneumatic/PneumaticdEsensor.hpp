#ifndef PNEUMATICDESENSOR_HPP_INCLUDED
#define PNEUMATICDESENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticdEsensor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 13 Mar 2013 16:55:00
//! @brief Pneumatic energy flow sensor
//! @ingroup PneumaticComponents
//!
//$Id$

using namespace hopsan;

class PneumaticdEsensor : public ComponentSignal
{
private:
     Port *mpPp1;
     double *mpPp1_dEp1, *mpOut;

public:
     static Component *Creator()
     {
        return new PneumaticdEsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic", "", Port::NotRequired);
        addOutputVariable("out", "EnergyFlow", "J/s", &mpOut);
     }

    void initialize()
     {
        mpPp1_dEp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::EnergyFlow);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpOut) = (*mpPp1_dEp1);
     }
};
#endif // PNEUMATICDESENSOR_HPP_INCLUDED
