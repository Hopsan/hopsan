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
     double *mpND_Tp1, *mpND_Tsensor;

public:
     static Component *Creator()
     {
        return new PneumaticTsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic");
        addOutputVariable("out", "Temperature", "K", &mpND_Tsensor);
     }

    void initialize()
     {
        mpND_Tp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::Temperature);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpND_Tsensor) = (*mpND_Tp1);
     }
};
#endif // PNEUMATICTSENSOR_HPP_INCLUDED
