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
//! @file   HydraulicDifferentialPressureSensor.hpp
//! @author Owen David <owen@sygalateia.com>
//! @date   5/1/2021
//!
//! @brief Contains a Hydraulic Differential Pressure Sensor Component
//!
//$Id$

#ifndef HYDRAULICDIFFERENTIALPRESSURESENSOR_HPP_INCLUDED
#define HYDRAULICDIFFERENTIALPRESSURESENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicDifferentialPressureSensor : public ComponentSignal
    {
    private:
        Port *mpP1, *mpP2;

        double *mpNp1, *mpNp2, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicDifferentialPressureSensor();
        }


        void configure()
        {
            mpP1 = addReadPort("P1", "NodeHydraulic", "", Port::NotRequired);
            mpP2 = addReadPort("P2", "NodeHydraulic", "", Port::NotRequired);
            addOutputVariable("out", "Differential Pressure (P1 - P2)", "Pressure", &mpOut);
        }


        void initialize()
        {
            mpNp1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpNp2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            simulateOneTimestep(); //Set initial output node value
        }


        void simulateOneTimestep()
        {
            (*mpOut) = (*mpNp1)-(*mpNp2);
        }
    };
}

#endif // HYDRAULICDIFFERENTIALPRESSURESENSOR_HPP_INCLUDED
