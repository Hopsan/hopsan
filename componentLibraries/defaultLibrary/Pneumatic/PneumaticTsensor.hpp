#ifndef PNEUMATICTSENSOR_HPP_INCLUDED
#define PNEUMATICTSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticTsensor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 13 Mar 2013 17:01:51
//! @brief Pneumatic tempreature sensor
//! @ingroup PneumaticComponents
//!
//$Id$

using namespace hopsan;

class PneumaticTsensor : public ComponentSignal
{
private:
     Port *mpPp1;
     double *mpPp1_T, *mpOut;

public:
     static Component *Creator()
     {
        return new PneumaticTsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic", "", Port::NotRequired);
        addOutputVariable("out", "Temperature", "K", &mpOut);
     }

    void initialize()
     {
        mpPp1_T=getSafeNodeDataPtr(mpPp1, NodePneumatic::Temperature);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpOut) = (*mpPp1_T);
     }
};
#endif // PNEUMATICTSENSOR_HPP_INCLUDED
