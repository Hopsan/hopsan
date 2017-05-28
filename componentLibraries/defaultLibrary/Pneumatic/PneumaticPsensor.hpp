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
