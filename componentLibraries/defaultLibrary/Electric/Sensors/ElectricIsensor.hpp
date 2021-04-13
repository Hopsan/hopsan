/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

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
        mpIn=addReadPort("Pel1","NodeElectric", "", Port::NotRequired);
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
