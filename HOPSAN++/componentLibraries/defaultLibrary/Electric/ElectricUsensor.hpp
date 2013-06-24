//!
//! @file ElectricUsensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @brief Sensor of electric voltage
//! @ingroup ElectricComponents
//!
//$Id$

#ifndef ELECTRICUSENSOR_HPP_INCLUDED
#define ELECTRICUSENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan{

class ElectricUsensor : public ComponentSignal
{
private:
     Port *mpIn;
     double *mpNDin, *mpNDout;

public:
     static Component *Creator()
     {
        return new ElectricUsensor();
     }

     void configure()
     {
        mpIn=addReadPort("Pel1","NodeElectric");
        addOutputVariable("Puout","Voltage","V", &mpNDout);
     }

    void initialize()
     {
        mpNDin = getSafeNodeDataPtr(mpIn,NodeElectric::Voltage);
        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        // Write in value to out port
        *mpNDout = *mpNDin;
     }
};
}
#endif // ELECTRICUSENSOR_HPP_INCLUDED
