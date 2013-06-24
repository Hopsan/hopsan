//!
//! @file ElectricIsensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @brief Sensor of electric current
//! @ingroup ElectricComponents
//!
//$Id$

#ifndef ELECTRICISENSOR_HPP_INCLUDED
#define ELECTRICISENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan{

class ElectricIsensor : public ComponentSignal
{
private:
     Port *mpIn;
     double *mpNDin, *mpNDout;

public:
     static Component *Creator()
     {
        return new ElectricIsensor();
     }

     void configure()
     {
        mpIn=addReadPort("Pel1","NodeElectric");
        addOutputVariable("Piout","Current","A",&mpNDout);
     }

     void initialize()
     {
         mpNDin=getSafeNodeDataPtr(mpIn, NodeElectric::Current);
         simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        *mpNDout = *mpNDin;
     }
};

}
#endif // ELECTRICISENSOR_HPP_INCLUDED
