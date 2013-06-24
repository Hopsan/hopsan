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
     double *mpND_pp1, *mpND_psensor;

public:
     static Component *Creator()
     {
        return new PneumaticPsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic");
        addOutputVariable("out", "Pressure", "Pa", &mpND_psensor);
     }

    void initialize()
     {
        mpND_pp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::Pressure);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpND_psensor) = (*mpND_pp1);
     }
};
#endif // PNEUMATICPSENSOR_HPP_INCLUDED
