//!
//! @file ElectricIsensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @brief Sensor of electric current
//! @ingroup ElectricComponents
//!

#ifndef ELECTRICISENSOR_HPP_INCLUDED
#define ELECTRICISENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan{

class ElectricIsensor : public ComponentSignal
{
private:
     Port *mpIn;
     Port *mpOut;
     double *mpNDin, *mpNDout;

public:
     static Component *Creator()
     {
        return new ElectricIsensor();
     }

     void configure()
     {
        mpIn=addReadPort("Pel1","NodeElectric");
        mpOut=addWritePort("Piout","NodeSignal", Port::NotRequired);
        disableStartValue(mpOut, NodeSignal::Value);
     }

     void initialize()
     {
         mpNDin=getSafeNodeDataPtr(mpIn, NodeElectric::Current);
         mpNDout=getSafeNodeDataPtr(mpOut, NodeSignal::Value);
         mpOut->setSignalNodeUnitAndDescription("A", "Current");
         simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        *mpNDout = *mpNDin;
     }
};

}
#endif // ELECTRICISENSOR_HPP_INCLUDED
